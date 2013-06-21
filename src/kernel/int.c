// #################################
// # Author: Timon Heim
// # Email: timon.heim at cern.ch
// # Project: Yarr
// # Description: Simple PCie Carrier Kernel driver
// # Comment: Original driver taken from Marcus Guillermo
// #          Modified for SPEC card
// ################################

#include <linux/version.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <stdbool.h>

#include "config.h"

#include "compat.h"

#include "pciDriver.h"

#include "common.h"

#include "int.h"

/**
 *
 * If IRQ-handling is enabled, this function will be called from specdriver_probe
 * to initialize the IRQ handling (maps the BARs)
 *
 */
int specdriver_probe_irq(specdriver_privdata_t *privdata)
{
	unsigned char int_pin, int_line;
	unsigned long bar_addr, bar_len, bar_flags;
	int i;
	int err;
    volatile void *bar4;
    volatile uint32_t *addr;

	for (i = 0; i < 6; i++)
		privdata->bars_kmapped[i] = NULL;

	for (i = 0; i < 3; i++) {
		bar_addr = pci_resource_start(privdata->pdev, i);
		bar_len = pci_resource_len(privdata->pdev, i);
		bar_flags = pci_resource_flags(privdata->pdev, i);

		/* check if it is a valid BAR, skip if not */
		if ((bar_addr == 0) || (bar_len == 0))
			continue;

		/* Skip IO regions (map only mem regions) */
		if (bar_flags & IORESOURCE_IO)
			continue;

		/* Check if the region is available */
		if ((err = pci_request_region(privdata->pdev, i, NULL)) != 0) {
			mod_info( "Failed to request BAR memory region.\n" );
			return err;
		}

		/* Map it into kernel space. */
		/* For x86 this is just a dereference of the pointer, but that is
		 * not portable. So we need to do the portable way. Thanks Joern!
		 */

		/* respect the cacheable-bility of the region */
		if (bar_flags & IORESOURCE_PREFETCH)
			privdata->bars_kmapped[i] = ioremap(bar_addr, bar_len);
		else
			privdata->bars_kmapped[i] = ioremap_nocache(bar_addr, bar_len);

		/* check for error */
		if (privdata->bars_kmapped[i] == NULL) {
			mod_info( "Failed to remap BAR%d into kernel space.\n", i );
			return -EIO;
		}
    }

    // Init GN4124
    //bar4 = privdata->bars_kmapped[4];
    //addr = (uint32_t*) bar4+0x800; // MSI_CONTROL
    //*addr = 0x000e5000;
    //mod_info("Activated MSI in GN4124!\n");

	/* Initialize the interrupt handler for this device */
	/* Initialize the wait queues */
	for (i = 0; i < SPECDRIVER_INT_MAXSOURCES; i++) {
		init_waitqueue_head(&(privdata->irq_queues[i]));
		atomic_set(&(privdata->irq_outstanding[i]), 0);
	}

	/* register interrupt handler */
	if ((err = request_irq(privdata->pdev->irq, specdriver_irq_handler, MODNAME, privdata)) != 0) {
		mod_info("Error registering the interrupt handler. Disabling interrupts for this device\n");
		return 0;
	}

	privdata->irq_enabled = 1;
	//mod_info("Registered Interrupt Handler at pin %i, line %i, IRQ %i\n", int_pin, int_line, privdata->pdev->irq );
	mod_info("Registered Interrupt Handler at IRQ %i\n", privdata->pdev->irq );

	return 0;
}

/**
 *
 * Frees/cleans up the data structures, called from specdriver_remove()
 *
 */
void specdriver_remove_irq(specdriver_privdata_t *privdata)
{
	/* Release the IRQ handler */
	if (privdata->irq_enabled != 0)
		free_irq(privdata->pdev->irq, privdata);

	specdriver_irq_unmap_bars(privdata);
}

/**
 *
 * Unmaps the BARs and releases them
 *
 */
void specdriver_irq_unmap_bars(specdriver_privdata_t *privdata)
{
	int i;

	for (i = 0; i < 6; i++) {
		if (privdata->bars_kmapped[i] == NULL)
			continue;

		iounmap((void*)privdata->bars_kmapped[i]);
		pci_release_region(privdata->pdev, i);
	}
}


/**
 *
 * Handles IRQs. At the moment, this acknowledges the card that this IRQ
 * was received and then increases the driver's IRQ counter.
 *
 * @see specdriver_irq_acknowledge
 *
 */
irqreturn_t specdriver_irq_handler(int irq, void *dev_id) {
    specdriver_privdata_t *privdata = (specdriver_privdata_t*) dev_id;
    mod_info("IRQ #%d handled!", irq);
   	atomic_inc(&(privdata->irq_outstanding[0]));
	wake_up_interruptible(&(privdata->irq_queues[0]));
    privdata->irq_count++;
    return IRQ_HANDLED;
}
