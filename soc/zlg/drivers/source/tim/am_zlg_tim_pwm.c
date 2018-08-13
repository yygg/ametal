/*******************************************************************************
*                                 AMetal
*                       ----------------------------
*                       innovating embedded platform
*
* Copyright (c) 2001-2018 Guangzhou ZHIYUAN Electronics Co., Ltd.
* All rights reserved.
*
* Contact information:
* web site:    http://www.zlg.cn/
*******************************************************************************/

/**
 * \file
 * \brief ��ʱ��TIM������PWM������ʵ��
 *
 * \internal
 * \par Modification history
 * - 1.00 17-04-21  nwt, first implementation
 * \endinternal
 */

#include "am_zlg_tim_pwm.h"
#include "am_int.h"
#include "am_clk.h"
#include "am_gpio.h"

/*******************************************************************************
* ��������
*******************************************************************************/

/** \brief ����PWM */
static int __tim_pwm_config (void          *p_drv,
                             int            chan,
                             unsigned long  duty_ns,
                             unsigned long  period_ns);

/** \brief ʹ��PWM��� */
static int __tim_pwm_enable (void *p_drv, int chan);

/** \brief ����PWM��� */
static int __tim_pwm_disable (void *p_drv, int chan);

/** \brief PWM�������� */
static const struct am_pwm_drv_funcs __g_tim_pwm_drv_funcs = {
    __tim_pwm_config,
    __tim_pwm_enable,
    __tim_pwm_disable,
};

/******************************************************************************/

/** \brief ����PWM */
static int __tim_pwm_config (void          *p_drv,
                             int            chan,
                             unsigned long  duty_ns,
                             unsigned long  period_ns)
{
    am_zlg_tim_pwm_dev_t *p_dev    = (am_zlg_tim_pwm_dev_t *)p_drv;
    amhw_zlg_tim_t       *p_hw_tim = (amhw_zlg_tim_t *)p_dev->p_devinfo->tim_regbase;

    uint32_t clkfreq;
    uint32_t period_c, duty_c, temp;
    uint16_t  pre_real = 1, pre_reg = 0;

    /* �������Ϸ� */
    if ((period_ns == 0) || (duty_ns > 4294967295UL) ||
        (period_ns > 4294967295UL) || (duty_ns > period_ns)) {
        return -AM_EINVAL;
    }

    /* ��Чͨ����Χ 0 ~ (channels_num - 1) */
    if (chan >= p_dev->p_devinfo->channels_num) {
        return -AM_EINVAL;
    }

    clkfreq = am_clk_rate_get(p_dev->p_devinfo->clk_num);

    /* ��������õ����Ǽ���ֵCNT, ��ʽns * 10e-9= cnt * (1/clkfrq) */
    period_c = (uint64_t)(period_ns) * (clkfreq) / (uint64_t)1000000000;
    duty_c   = (uint64_t)(duty_ns)   * (clkfreq) / (uint64_t)1000000000;

   {

        /* ������С��65536ʱ������Ƶ(ֵΪ1,1����Ϊ1��Ƶ) */
        temp = period_c / 65536 + 1;

        /* 16λ��ʱ����Ҫ����ȡ�ú��ʵķ�Ƶֵ */
        for (pre_real = 1; pre_real < temp; ) {
             pre_reg++;           /* ����д��Ĵ����ķ�Ƶֵ0,1,2,... */
             pre_real++;          /* ��Ƶ�� */
        }
    }

    /* д���Ƶֵ */
    amhw_zlg_tim_prescale_set(p_hw_tim, pre_reg);

    /* ���¼���PWM�����ڼ�����Ƶ�� */
    period_c = period_c / pre_real;
    duty_c = duty_c / pre_real;

    /* ����һ�����ؼĴ�����PWM�����е����ڶ���ͬ */
    amhw_zlg_tim_arr_set(p_hw_tim, period_c - 1);

    /* ʹ�ܶ�ARRԤװ�� */
    amhw_zlg_tim_arpe_enable(p_hw_tim);

    /* Low level time */
    duty_c = period_c - duty_c;

    /* ����ֵΪPWM����͵�ƽ��ʱ�� */
    amhw_zlg_tim_ccr_ouput_reload_val_set(p_hw_tim, duty_c - 1, chan);

    /* ѡ���ͨ��Ϊ��� */
    amhw_zlg_tim_ccs_set(p_hw_tim, 0, chan);

    /* ѡ���ͨ����PWMģʽ */
    amhw_zlg_tim_ocm_set(p_hw_tim, p_dev->p_devinfo->pwm_mode, chan);

    /* PWM���ͨ��Ԥװ��ʹ�� */
    amhw_zlg_tim_ccs_ocpe_enable(p_hw_tim, chan);

    if ((p_dev->p_devinfo->tim_type == AMHW_ZLG_TIM_TYPE0) ||
        (p_dev->p_devinfo->tim_type == AMHW_ZLG_TIM_TYPE2) ||
        (p_dev->p_devinfo->tim_type == AMHW_ZLG_TIM_TYPE3)) {

        /* ����CR2�Ĵ���ȷ��N������ */
    }

    /* ���ñȽ����ͨ���ߵ�ƽ������Ч */
    amhw_zlg_tim_ccp_output_set(p_hw_tim, p_dev->p_devinfo->ocpolarity, chan);

    return AM_OK;
}

/**
 * \brief ʹ��PWMͨ�����
 */
static int __tim_pwm_enable (void *p_drv, int chan)
{
    am_zlg_tim_pwm_dev_t    *p_dev    = (am_zlg_tim_pwm_dev_t *)p_drv;
    amhw_zlg_tim_t          *p_hw_tim = (amhw_zlg_tim_t *)p_dev->p_devinfo->tim_regbase;
    am_zlg_tim_pwm_ioinfo_t *p_ioinfo = p_dev->p_devinfo->p_ioinfo;

    /* ��Чͨ����Χ 0 ~ (channels_num - 1) */
    if (chan >= p_dev->p_devinfo->channels_num) {
        return -AM_EINVAL;
    }

    am_gpio_pin_cfg(p_ioinfo[chan].gpio, p_ioinfo[chan].func);

    /* ʹ��ͨ��PWM��� */
    amhw_zlg_tim_cce_output_enable(p_hw_tim, chan);

    /* �߼���ʱ��ʹ�������MOE */
    if ((p_dev->p_devinfo->tim_type == AMHW_ZLG_TIM_TYPE0) ||
        (p_dev->p_devinfo->tim_type == AMHW_ZLG_TIM_TYPE2) ||
        (p_dev->p_devinfo->tim_type == AMHW_ZLG_TIM_TYPE3)) {

        amhw_zlg_tim_bdtr_enable(p_hw_tim, AMHW_ZLG_TIM_MOE);
    }

    /* ���������¼������³�ʼ��Prescaler��������Repetition������ */
    amhw_zlg_tim_egr_set(p_hw_tim, AMHW_ZLG_TIM_UG);

    if (amhw_zlg_tim_status_flg_get(p_hw_tim, AMHW_ZLG_TIM_UG) != 0) {

        /* ���¶�ʱ��ʱ����������¼�,�����־λ */
        amhw_zlg_tim_status_flg_clr(p_hw_tim, AMHW_ZLG_TIM_UG);
    }

    /* ʹ�ܶ�ʱ��TIM�������� */
    amhw_zlg_tim_enable(p_hw_tim);

    return AM_OK;
}

/**
 * \brief ����PWMͨ�����
 */
static int __tim_pwm_disable (void *p_drv, int chan)
{
    am_zlg_tim_pwm_dev_t    *p_dev    = (am_zlg_tim_pwm_dev_t *)p_drv;
    amhw_zlg_tim_t          *p_hw_tim = (amhw_zlg_tim_t *)p_dev->p_devinfo->tim_regbase;
    am_zlg_tim_pwm_ioinfo_t *p_ioinfo = p_dev->p_devinfo->p_ioinfo;

    /* ��Чͨ����Χ 0 ~ (channels_num - 1) */
    if (chan >= p_dev->p_devinfo->channels_num) {
        return -AM_EINVAL;
    }

    am_gpio_pin_cfg(p_ioinfo[chan].gpio, p_ioinfo[chan].dfunc);

    /* ���ܶ�ʱ��TIM�������� */
    amhw_zlg_tim_disable(p_hw_tim);

    /* ����ͨ��PWM��� */
    amhw_zlg_tim_cce_output_disable(p_hw_tim, chan);

    /* �߼���ʱ�����������MOE */
    if ((p_dev->p_devinfo->tim_type == AMHW_ZLG_TIM_TYPE0) ||
        (p_dev->p_devinfo->tim_type == AMHW_ZLG_TIM_TYPE2) ||
        (p_dev->p_devinfo->tim_type == AMHW_ZLG_TIM_TYPE3)) {

        amhw_zlg_tim_bdtr_disable(p_hw_tim, AMHW_ZLG_TIM_MOE);
    }

    return AM_OK;
}

/**
  * \brief pwm��ʼ��
  */
void __tim_pwm_init (amhw_zlg_tim_t     *p_hw_tim,
                     amhw_zlg_tim_type_t type)
{
    if ((type == AMHW_ZLG_TIM_TYPE0) || (type == AMHW_ZLG_TIM_TYPE1)) {

        /* ���ض���ģʽ */
        amhw_zlg_tim_cms_set(p_hw_tim, 0);

        /* ���ϼ��� */
        amhw_zlg_tim_dir_set(p_hw_tim, 0);
    }

    /* ����ʱ�ӷָ�(����ȷ�������˲���������ʱ��Ƶ��,Ĭ�� Fdts = Fck_in */
    amhw_zlg_tim_ckd_set(p_hw_tim, 0);

    /* ��������� */
    amhw_zlg_tim_count_set(p_hw_tim, 0);

    /* ���÷�Ƶ�� */
    amhw_zlg_tim_prescale_set(p_hw_tim, 0x00);

    /* ���������¼� */
    amhw_zlg_tim_udis_enable(p_hw_tim);
}

/**
  * \brief tim pwm�����ʼ��
  */
am_pwm_handle_t am_zlg_tim_pwm_init (am_zlg_tim_pwm_dev_t           *p_dev,
                                     const am_zlg_tim_pwm_devinfo_t *p_devinfo)
{
    amhw_zlg_tim_t  *p_hw_tim = NULL;

    if (p_dev == NULL || p_devinfo == NULL) {
        return NULL;
    }

    if (p_devinfo->pfn_plfm_init) {
        p_devinfo->pfn_plfm_init();
    }

    p_dev->p_devinfo        = p_devinfo;
    p_hw_tim                = (amhw_zlg_tim_t *)p_dev->p_devinfo->tim_regbase;
    p_dev->pwm_serv.p_funcs = (struct am_pwm_drv_funcs *)&__g_tim_pwm_drv_funcs;
    p_dev->pwm_serv.p_drv   = p_dev;

    __tim_pwm_init(p_hw_tim, p_devinfo->tim_type);

    return &(p_dev->pwm_serv);
}

void am_zlg_tim_pwm_deinit (am_pwm_handle_t handle)
{

    am_zlg_tim_pwm_dev_t *p_dev    = (am_zlg_tim_pwm_dev_t *)handle;
    amhw_zlg_tim_t       *p_hw_tim = NULL;

    if (p_dev == NULL ) {
        return;
    }

    p_hw_tim   = (amhw_zlg_tim_t *)p_dev->p_devinfo->tim_regbase;

    /* ��������� */
    amhw_zlg_tim_count_set(p_hw_tim, 0);

    /* ���ܶ�ʱ��TIM�������� */
    amhw_zlg_tim_disable(p_hw_tim);

    p_dev->pwm_serv.p_drv = NULL;

    if (p_dev->p_devinfo->pfn_plfm_deinit) {
        p_dev->p_devinfo->pfn_plfm_deinit();
    }
}

/* end of file */