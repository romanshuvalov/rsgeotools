#ifndef RSDEBUG_H_INCLUDED
#define RSDEBUG_H_INCLUDED

#ifndef STREETS_GAME

    #define DEBUG10(a)      ((void)0)
    #define DEBUG10f(...)   ((void)0)

    #define DEBUG20(a)      ((void)0)
    #define DEBUG20f(...)   ((void)0)

    #define DEBUG30(a)      ((void)0)
    #define DEBUG30f(...)   ((void)0)


    #define DEBUGR(a)
    #define DEBUGRf(...)


	#define CHECK_GL ((void)0)

	#define rs_app_assert_memory(...) ((void)0)
	#define rs_app_assert_memory_adv(...) ((void)0)

	#define RS_UNUSED_PARAM(...) ((void)0)

	#define rvt_app_increase_threads_count(...) ((void)0)

	#define rs_show_message(...) ((void)0)
	#define rs_show_message_sprintf(...) ((void)0)

	#define rs_critical_alert_and_halt(...) ((void)0)
	#define rs_critical_alert_and_halt_sprintf(...) ((void)0)

	#define rvt_app_exit(...) ((void)0)
	
#endif

#endif // RSDEBUG_H_INCLUDED
