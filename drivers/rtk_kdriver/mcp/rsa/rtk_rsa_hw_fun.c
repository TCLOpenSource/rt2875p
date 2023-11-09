#include <linux/version.h>
#include <crypto/algapi.h>
#include <rtk_kdriver/mcp/rsa/rtk_rsa_api.h>
#include <rtk_kdriver/mcp/rsa/rtk_rsa_hw_fun.h>
#include "mcp/rtk_mcp_reg.h"
#include "rtk_mcp_wrapper.h"
#include <linux/delay.h>

extern bool is_cp_clken(void);

#define ciL		(sizeof(mpi_limb_t))	/* chars in limb  */
#define biL		(ciL << 3)			/* bits  in limb  */
#define BITS_TO_LIMBS(i) ((i) / biL + ((i) % biL != 0))

//#define REG_DEBUG
#ifndef REG_DEBUG
#define WRITE_REG32(addr, x)  rtd_outl(addr, x)
#define READ_REG32(addr)      rtd_inl(addr)
#else
static inline void WR_DEBUG(const char *reg_name, unsigned long reg, unsigned int x)
{
    RSA_INFO("W: %-30s 0x%lx: 0x%x", reg_name, (unsigned long)reg, x);
    rtd_outl(reg, x);
}
static inline unsigned int RD_DEBUG(const char *reg_name, unsigned long reg)
{
    unsigned int x = rtd_inl(reg);
    RSA_INFO("R: %-30s 0x%lx: 0x%x", reg_name, (unsigned long)reg, x);
    return x;
}
#define WRITE_REG32(addr, x)         WR_DEBUG(#addr, addr, x)
#define READ_REG32(addr)             RD_DEBUG(#addr, addr)
#endif

static DEFINE_MUTEX(hw_rsa_mutex);

/*-----------------------------------------------------------------------------*/
/*====================== For HW RSA utility function ==========================*/
/*-----------------------------------------------------------------------------*/
#if 0
static int _check_msb_bit (MPI key)
{
    unsigned int check_bit;
    int i, j;

    if (key == NULL)
        return -1;

    for ( i = (key->nlimbs - 1); i >= 0; i-- ) {
        check_bit = 0x80000000;

        for ( j = 0; j < 32; j++ ) {
            if ( (key->d[i] & check_bit ) == check_bit ) {
                return ( ( ( i + 1 ) << 5 ) - ( j + 1 ) );
            }

            check_bit >>= 1;
        }
    }
    return 0;
}
#endif

static int _is_bit_set (MPI key, int n)
{
    if (key == NULL)
        return -1;

     if( key->nlimbs * biL <= n )
        return( 0 );

    return( ( key->d[n / biL] >> ( n % biL ) ) & 0x01 );
}

static int hwsu_get_data (void *addr, MPI key, UINT8 type)
{
    int i, j, max_idx;
    int ret = 0;
    unsigned int *reg = (unsigned int *) addr;
    unsigned int *regM = (unsigned int *) (addr + 0x800);

    if (type == TYPE_HW_MONT)
        max_idx = HW_MAX_SIZE; //Max: 64 * 32 = 2048 bit
    else
        max_idx = 12; //Max: 13 * 32 = 416 bit

    /* Skip MSB zero */
    i = max_idx;
    while (1) {
        if (i < 64)
            if (i && (rtd_inl((unsigned long)(reg + i)) == 0))
                i--;
            else
                break;
        else
            if (i && (rtd_inl((unsigned long)(regM + i - 64)) == 0))
                i--;
            else
                break;
    }
    /* Fill BI */
    if (biL == 32) {
        key->nlimbs = i + 1;
        j = 1;
        for (; i >= 0; i--) {
            unsigned int *tmp_reg;
            if (i < 64)
                tmp_reg = reg;
            else
                tmp_reg = regM - 64;
            key->d[key->nlimbs - j] = rtd_inl((unsigned long)(tmp_reg + i));
            j++;
        }
    } else if (biL == 64) {
        i = (i % 2) ? i : i + 1;
        key->nlimbs = (i + 1)/2;
        j = 1;
        for (; i >= 0; i-=2) {
            unsigned int *tmp_reg;
            //unsigned int value;
            if (i < 64)
                tmp_reg = reg;
            else
                tmp_reg = regM - 64;
            key->d[key->nlimbs - j] = READ_REG32((unsigned long)(tmp_reg + i));
            key->d[key->nlimbs - j] = key->d[key->nlimbs - j] << 32;
            key->d[key->nlimbs - j] |= READ_REG32((unsigned long)(tmp_reg + i - 1));
            j++;
        }
    }
    
    return ret;
}
//-----------------------------------------------------------------------------
static void hwsu_set_data (void *addr, MPI key, UINT8 type)
{
    //int i, max_idx;
    int i;
    unsigned int *reg = (unsigned int *) addr;
    unsigned int *regM = (unsigned int *) (addr + 0x800);
    unsigned int *tmp_reg;

    //if (type == TYPE_HW_MONT)
    //    max_idx = HW_MAX_SIZE; //Max: 64 * 32 = 2048 bit
    //else
    //    max_idx = 12; //Max: 13 * 32 = 416 bit
    if (biL == 32) {
        for (i = 0; i < key->nlimbs; i++) {
            if (i > 64 - 1)
                tmp_reg = regM - 64;
            else
                tmp_reg = reg;
            rtd_outl((unsigned long)(tmp_reg + i), key->d[i]);
        }
    } else if (biL == 64) {
        for (i = 0; i < key->nlimbs; i++) {
            if (i > 32 - 1)
                tmp_reg = regM - 64;
            else
                tmp_reg = reg;
            WRITE_REG32((unsigned long)(tmp_reg + i*2), key->d[i] & 0xffffffff);
            WRITE_REG32((unsigned long)(tmp_reg + i*2 + 1), key->d[i] >> 32);
        }
        i = i << 1;
    }
    return;
}
//-----------------------------------------------------------------------------
static void hwsu_set_bitnum (unsigned int bsize)
{
    rtd_outl((REG_RSA_NUM), (bsize) - 1);
}
//-----------------------------------------------------------------------------
static int hwsu_chk_mont_engine_ready (void)
{
    int ret = 0;
    unsigned long timeout = jiffies + HZ; /* timeout in 1s */
    while (rtd_inl(REG_RSA_CTRL) & RSA_go) {
        if(timeout <= jiffies) {
            ret = -1;
            RSA_ERR("[%s]%d: HW RSA check mont engine ready timeout!!\n", __FUNCTION__, __LINE__);
            break;
        }
    }
    return ret;
}
//-----------------------------------------------------------------------------
static void hwsu_mont_engine_go (void)
{
    rtd_outl((REG_RSA_CTRL), RSA_go);
}
//----------------------------------------------------------------------------


#define HW_SET_RSA_A(bi)  {hwsu_chk_mont_engine_ready();hwsu_set_data((void *)REG_RSA_A,bi,TYPE_HW_MONT);}
#define HW_SET_RSA_X(bi)  {hwsu_chk_mont_engine_ready();hwsu_set_data((void *)REG_RSA_X,bi,TYPE_HW_MONT);}
#define HW_SET_RSA_Y(bi)  {hwsu_chk_mont_engine_ready();hwsu_set_data((void *)REG_RSA_Y,bi,TYPE_HW_MONT);}
#define HW_SET_RSA_MOD(bi)  {hwsu_chk_mont_engine_ready();hwsu_set_data((void *)REG_RSA_M,bi,TYPE_HW_MONT);}

#define HW_GET_RSA_A(bi)  {hwsu_chk_mont_engine_ready();hwsu_get_data((void *)REG_RSA_A,bi,TYPE_HW_MONT);}
#define HW_GET_RSA_X(bi)  {hwsu_chk_mont_engine_ready();hwsu_get_data((void *)REG_RSA_X,bi,TYPE_HW_MONT);}
#define HW_GET_RSA_Y(bi)  {hwsu_chk_mont_engine_ready();hwsu_get_data((void *)REG_RSA_Y,bi,TYPE_HW_MONT);}
#define HW_GET_RSA_MOD(bi)  {hwsu_chk_mont_engine_ready();hwsu_get_data((void *)REG_RSA_M,bi,TYPE_HW_MONT);}

static inline void hwsu_mont_AA_go(void)
{
    rtd_outl((REG_RSA_CTRL), RSA_go | RSA_A2X | RSA_X2Y);
    hwsu_chk_mont_engine_ready();
}

static inline void hwsu_mont_AY_go(void)
{
    rtd_outl((REG_RSA_CTRL), RSA_go | RSA_A2X);
    hwsu_chk_mont_engine_ready();
}

static inline void hwsu_mont_XY_go(void)
{
    rtd_outl((REG_RSA_CTRL), RSA_go);
    hwsu_chk_mont_engine_ready();
}

static int hwsu_mont (MPI resl, MPI a, MPI b, MPI m, unsigned int bsize)
{
    int ret = 0;
    if((ret = hwsu_chk_mont_engine_ready()) != 0)
        return ret;
    rtd_outl(REG_RSA_CLEAR, RSA_clear_all);

    hwsu_set_data((void *)REG_RSA_X, a, TYPE_HW_MONT);
    hwsu_set_data((void *)REG_RSA_Y, b, TYPE_HW_MONT);
    hwsu_set_data((void *)REG_RSA_M, m, TYPE_HW_MONT);

    hwsu_set_bitnum(bsize);

    hwsu_mont_engine_go();

    if((ret = hwsu_chk_mont_engine_ready()) != 0)
        return ret;

    ret = hwsu_get_data((void *)REG_RSA_A, resl, TYPE_HW_MONT);

    return  ret;
}

int rtk_rsa_get_r2(MPI mod, MPI r2)
{
    int ret = 0;
    MPI one = NULL;
    MPI base = NULL;
    
    if(mod == NULL){
        ret = -1;
        goto ERR;
    }
    
    one = mpi_alloc(1);
    if (one == NULL) {
        ret = -ENOMEM;
        goto ERR;
    }
	
    base = mpi_alloc(mod->nlimbs * 2 + 1);
    if (base == NULL) {
        ret = -ENOMEM;
        goto FREE_ONE;
    }
    
    one->d[0] = 1;
    one->nlimbs = 1;
    memset(base->d, 0, base->alloced * sizeof(mpi_limb_t));
    base->d[mod->nlimbs * 2] = 1;
    base->nlimbs = mod->nlimbs * 2 + 1;
    ret = mpi_powm(r2, base, one, mod);

    mpi_free(base);
FREE_ONE:
    mpi_free(one);
ERR:
    return ret;
}

void rtk_rsa_release_scpu(void)
{
    SET_S_RSA_SEL(0);
}
int rtk_rsa_select_scpu(void)
{
    unsigned int engine_sel = GET_S_RSA_SEL();
    unsigned int count = 0;
    SET_S_RSA_SEL((engine_sel & ~(S_RSA_SEL_ENGINE_MASK)) | S_RSA_SEL_ENGINE);
    while((GET_S_RSA_ST() & (S_RSA_ENGINE_READY_MASK)) != S_RSA_ENGINE_READY) {
        usleep_range(200, 400);
        /* scpu RSA can't ready */
        if (count++ > 5000) {
            rtk_rsa_release_scpu();
            RSA_ERR("get RSA scpu fail");
            return -EUSERS;
        }
    }
    return 0;
}
/*
*mpi_resize not export.If change the flow without mpi_resize,need to modify the whole flow.So just
*Copy the mpi_resize and rename.
*/

int rtk_mpi_resize(MPI a, unsigned nlimbs)
{
	void *p;

	if (nlimbs <= a->alloced)
		return 0;	/* no need to do it */

	if (a->d) {
		p = kcalloc(nlimbs, sizeof(mpi_limb_t), GFP_KERNEL);
		if (!p)
			return -ENOMEM;
		memcpy(p, a->d, a->alloced * sizeof(mpi_limb_t));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0))
		kfree_sensitive(a->d);
#else
		kzfree(a->d);
#endif
		a->d = p;
	} else {
		a->d = kcalloc(nlimbs, sizeof(mpi_limb_t), GFP_KERNEL);
		if (!a->d)
			return -ENOMEM;
	}
	a->alloced = nlimbs;
	return 0;
}

int hwsu_mont_exp_fast(
    MPI out, MPI in, MPI mod, MPI r2, MPI key)
{
    int ret = -1;
    int i = 0;

    if(out == NULL || in == NULL || mod == NULL
        || key == NULL || r2 == NULL)
        return ret;

    if (!is_cp_clken())
        return -ENXIO;
	
    mutex_lock(&hw_rsa_mutex);
    ret = rtk_rsa_select_scpu();
    if (ret) {
        mutex_unlock(&hw_rsa_mutex);
        return ret;
    }
    rtk_mpi_resize(out, mod->nlimbs);
    rtk_mpi_resize(in, mod->nlimbs);
    out->d[0] = 1;
    out->nlimbs = 1;
    //hwsu_mont(base,base,r2,mod,bsize);
    hwsu_mont(in, in, r2, mod, mod->nbits/32);
    //hwsu_mont(resl,resl,r2,mod,bsize);
    rtd_outl((REG_RSA_CLEAR), RSA_clear_x);
    HW_SET_RSA_X(out);
    hwsu_mont_XY_go();
    rtd_outl((REG_RSA_CLEAR), RSA_clear_y);
    HW_SET_RSA_Y(in);
    for (i = mpi_get_nbits(key); i >= 0; i--) {
        //hwsu_mont(resl,resl,resl,mod,bsize);
        hwsu_mont_AA_go();
        if (_is_bit_set(key, i) == 1) {
            //hwsu_mont(resl,resl,base,mod,bsize);
            hwsu_mont_AY_go();
        }
    }
    //hwsu_mont(resl,resl,temp_1,mod,bsize);
    rtd_outl((REG_RSA_CLEAR), RSA_clear_y | RSA_clear_x);
    out->d[0] = 1;
    out->nlimbs = 1;
    HW_SET_RSA_Y(out);
    hwsu_mont_AY_go();
    HW_GET_RSA_A(out);
    ret = 0;
    rtk_rsa_release_scpu();
    mutex_unlock(&hw_rsa_mutex);
    return ret;
}

extern void rsa_module_test(void);

int  rtk_rsa_core_init(void)
{
#ifdef CONFIG_RTK_KDRV_MCP_RSA_TEST
    rsa_module_test();
#endif
    return 0;
}
int rtk_rsa_core_exit(void)
{
    return 0;
}
