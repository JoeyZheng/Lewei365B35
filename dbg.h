#ifndef __DBG_H__
#define __DBG_H__

#define ENABLEDBG 0

#if ENABLEDBG
#define DBGPRINT printf
#else
#define DBGPRINT
#endif

#endif /* __DBG_H_ */
