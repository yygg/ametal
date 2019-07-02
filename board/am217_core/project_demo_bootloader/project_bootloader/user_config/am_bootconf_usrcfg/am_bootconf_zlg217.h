#include "am_boot_flash.h"
#include "am_boot_firmware_recv.h"
#include "am_boot_enter_check.h"
#include "am_boot_firmware.h"
#include "am_boot_autobaud_soft.h"
#include "am_boot_msg.h"

/** \brief ����BootLoader��׼�ӿ�ʵ����ʼ��*/
int am_zlg217_boot_single_inst_init(void);

/** \brief �����̼����ʵ����ʼ�� */
am_boot_firmware_handle_t am_zlg217_boot_single_firmware_flash_inst_init(void);

/******************************************************************************/

/** \brief ˫���̼����ʵ����ʼ�� */
am_boot_firmware_handle_t am_zlg217_boot_double_firmware_flash_inst_init(void);
/** \brief ��Ϣ����־�ķ�ʽ��ʵ����ʼ�� */
am_boot_msg_handle_t am_zlg217_boot_msg_flag_inst_init(void);
/** \brief ˫��BootLoader��׼�ӿ�ʵ����ʼ�� */
int am_zlg217_boot_double_inst_init(void);


/******************************************************************************/

/** \brief ʵ����ʼ��������Զ������ʷ����� */
am_boot_autobaud_handle_t am_zlg217_boot_autobaud_inst_init (void);
/** \brief flashʵ����ʼ�� */
am_boot_flash_handle_t am_zlg217_boot_kft_flash_inst_init(void);
/** \brief kft��λ��ʵ����ʼ�� */
int am_zlg217_boot_kft_inst_init(void);

/* end of file */