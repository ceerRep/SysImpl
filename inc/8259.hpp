#ifndef _8259_hpp

#define _8259_hpp

extern "C"
{
    void PIC_sendEOI(unsigned char irq);
    void PIC_remap(int offset1, int offset2);
    void IRQ_set_mask(unsigned char IRQline);
    void IRQ_clear_mask(unsigned char IRQline);
}

#endif
