#ifndef RENZAN_SYSCALL_H__
#define RENZAN_SYSCALL_H__


#define AS_CURRENT 0

#ifdef _KERNEL



void syscall_def(void);
void syscall_init(void);

#endif /* _KERNEL */

#endif /* RENZAN_SYSCALL_H__ */
