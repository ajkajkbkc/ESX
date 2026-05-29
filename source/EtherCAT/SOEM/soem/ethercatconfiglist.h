/*
 * Licensed under the GNU General Public License version 2 with exceptions. See
 * LICENSE file in the project root for full license information
 */

/** \file
 * \brief
 * DEPRECATED Configuration list of known EtherCAT slave devices.
 *
 * If a slave is found in this list it is configured according to the parameters
 * in the list. Otherwise the configuration info is read directly from the slave
 * EEPROM (SII or Slave Information Interface).
 */

#ifndef _ethercatconfiglist_
#define _ethercatconfiglist_

#ifdef __cplusplus
extern "C"
{
#endif

/*
   explanation of dev:
    1: static device with no IO mapping ie EK1100
    2: input device no mailbox ie simple IO device
    3: output device no mailbox
    4: input device with mailbox configuration
    5: output device with mailbox configuration
    6: input/output device no mailbox
    7: input.output device with mailbox configuration
*/
#define EC_CONFIGEND 0xffffffff

/** Slave configuration structure */
typedef struct
{
   /** Manufacturer code of slave */
   uint32           man;
   /** ID of slave */
   uint32           id;
   /** Readable name */
   char             name[EC_MAXNAME + 1];
   /** Data type */
   uint8            Dtype;
   /** Input bits */
   uint16            Ibits;
   /** Output bits */
   uint16           Obits;
   /** SyncManager 2 address */
   uint16           SM2a;
   /** SyncManager 2 flags */
   uint32           SM2f;
   /** SyncManager 3 address */
   uint16           SM3a;
   /** SyncManager 3 flags */
   uint32           SM3f;
   /** FMMU 0 activation */
   uint8            FM0ac;
   /** FMMU 1 activation */
   uint8            FM1ac;
} ec_configlist_t;

extern ec_configlist_t ec_configlist[3];
#ifdef __cplusplus
}
#endif

#endif
