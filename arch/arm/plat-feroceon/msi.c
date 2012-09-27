/*
 * arch/arm/plat-feroceon/msi.c
 *
 * Marvell KW2 SoC MSI, MSI-X handling.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/pci.h>
#include <linux/msi.h>
#include <asm/mach/irq.h>
#include <asm/irq.h>
#include "mvOs.h"

static DECLARE_BITMAP(msi_irq_in_use, NR_MSI_IRQS);

void msi_irq_handler(unsigned int irq, struct irq_desc *desc)
{
	int b;
	unsigned long status;

	/* Read Inbound Doorbell (Host-to-CPU) register and
	 * call ISR for each active interrupt.
	 */
	/* status = MV_REG_READ(MV_IN_DRBELL_CAUSE_REG) & 0xFFFF0000; */
	status = MV_REG_READ(MV_IN_DRBELL_CAUSE_REG);
	if (!status)
		return;

	b = find_first_bit(&status, 32);

	/* write back to clear bit (to acknowledge interrupt) */
	MV_REG_WRITE(MV_IN_DRBELL_CAUSE_REG, ~(1 << b));
	status = MV_REG_READ(MV_IN_DRBELL_CAUSE_REG);

	generic_handle_irq(IRQ_MSI_START + b);
}

void __init kw2_msi_init(void)
{
	unsigned long temp;

	set_irq_chained_handler(IN_DRBELL_IRQ_NUM, msi_irq_handler);

	/* Unmask private doorbells 16-31 */
	/* temp = MV_REG_READ(MV_IN_DRBELL_MASK_REG) | (0xFFFF0000); */
	temp = MV_REG_READ(MV_IN_DRBELL_MASK_REG) | 0xFFFFFFFF;
	MV_REG_WRITE(MV_IN_DRBELL_MASK_REG, temp);
}

/*
 * Dynamic irq allocation and deallocation
 */
int create_irq(void)
{
	int irq, pos;

again:
	pos = find_first_zero_bit(msi_irq_in_use, NR_MSI_IRQS);
	irq = IRQ_MSI_START + pos;
	if (irq > NR_IRQS)
		return -ENOSPC;
	/* test_and_set_bit operates on 32-bits at a time */
	if (test_and_set_bit(pos, msi_irq_in_use))
		goto again;

	dynamic_irq_init(irq);
	/* irq_clear_status_flags(irq, IRQ_NOREQUEST); */
	irq_desc[irq].status |= IRQ_LEVEL;
	set_irq_flags(irq, IRQF_VALID);

	return irq;
}

void destroy_irq(unsigned int irq)
{
	int pos = irq - IRQ_MSI_START;
	dynamic_irq_cleanup(irq);
	clear_bit(pos, msi_irq_in_use);
}

void arch_teardown_msi_irq(unsigned int irq)
{
	destroy_irq(irq);
}

static void msi_nop(unsigned int irq)
{
	return;
}

struct irq_chip msi_irq_chip = {
	.name			= "kw2_msi_irq",
	.ack			= msi_nop,
	.enable			= unmask_msi_irq,
	.disable		= mask_msi_irq,
	.mask			= mask_msi_irq,
	.unmask			= unmask_msi_irq,
};

int arch_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc)
{
	int irq;
	int msi_irq;
	struct msi_msg msg;

	irq = create_irq();
	if (irq < 0)
		return irq;
	set_irq_msi(irq, desc);

	msi_irq = irq - IRQ_MSI_START;

	msg.address_hi = 0x0;
	msg.address_lo = (INTER_REGS_BASE | MV_IN_DRBELL_CAUSE_REG);
	msg.data = (0x1 << msi_irq);

	write_msi_msg(irq, &msg);
	set_irq_chip_and_handler(irq, &msi_irq_chip, handle_edge_irq);

	return 0;
}
