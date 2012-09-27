#ifndef _FEROCEON_KW2_MSI_H_
#define _FEROCEON_KW2_MSI_H_

#ifdef CONFIG_PCI_MSI
void kw2_msi_init(void);
#else
static inline void kw2_msi_init(void)
{
	return;
}
#endif

#endif /* _FEROCEON_KW2_MSI_H_ */
