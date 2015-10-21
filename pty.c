//#include	"config.h"
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/file.h>

#define HAVE_SYS_WAIT_H

#include	<fcntl.h>
#include	<errno.h>
#include	<pwd.h>
#ifdef HAVE_SYS_IOCTL_H
#include	<sys/ioctl.h>
#endif
#ifdef HAVE_LIBUTIL_H
#include	<libutil.h>
#endif
#ifdef HAVE_PTY_H
#include	<pty.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#else
#define WIFSTOPPED(status)    (((status) & 0xff) == 0x7f)
#endif
#include <ctype.h>

#include <mruby.h>
#include <mruby/variable.h>
#include <mruby/error.h>
#include <mruby/data.h>

//#include "rubyio.h"
//#include "util.h"

#include <signal.h>
#ifdef HAVE_SYS_STROPTS_H
#include <sys/stropts.h>
#endif

#include <stdlib.h>
#include <pthread.h>

#define HAVE_UNISTD_H

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#define	DEVICELEN	16

#if !defined(HAVE_OPENPTY)
#if defined(__hpux)
static const
char	MasterDevice[] = "/dev/ptym/pty%s",
	SlaveDevice[] =  "/dev/pty/tty%s",
	*const deviceNo[] = {
		"p0","p1","p2","p3","p4","p5","p6","p7",
		"p8","p9","pa","pb","pc","pd","pe","pf",
		"q0","q1","q2","q3","q4","q5","q6","q7",
		"q8","q9","qa","qb","qc","qd","qe","qf",
		"r0","r1","r2","r3","r4","r5","r6","r7",
		"r8","r9","ra","rb","rc","rd","re","rf",
		"s0","s1","s2","s3","s4","s5","s6","s7",
		"s8","s9","sa","sb","sc","sd","se","sf",
		"t0","t1","t2","t3","t4","t5","t6","t7",
		"t8","t9","ta","tb","tc","td","te","tf",
		"u0","u1","u2","u3","u4","u5","u6","u7",
		"u8","u9","ua","ub","uc","ud","ue","uf",
		"v0","v1","v2","v3","v4","v5","v6","v7",
		"v8","v9","va","vb","vc","vd","ve","vf",
		"w0","w1","w2","w3","w4","w5","w6","w7",
		"w8","w9","wa","wb","wc","wd","we","wf",
		0,
	};
#elif defined(_IBMESA)  /* AIX/ESA */
static const
char	MasterDevice[] = "/dev/ptyp%s",
  	SlaveDevice[] = "/dev/ttyp%s",
	*const deviceNo[] = {
"00","01","02","03","04","05","06","07","08","09","0a","0b","0c","0d","0e","0f",
"10","11","12","13","14","15","16","17","18","19","1a","1b","1c","1d","1e","1f",
"20","21","22","23","24","25","26","27","28","29","2a","2b","2c","2d","2e","2f",
"30","31","32","33","34","35","36","37","38","39","3a","3b","3c","3d","3e","3f",
"40","41","42","43","44","45","46","47","48","49","4a","4b","4c","4d","4e","4f",
"50","51","52","53","54","55","56","57","58","59","5a","5b","5c","5d","5e","5f",
"60","61","62","63","64","65","66","67","68","69","6a","6b","6c","6d","6e","6f",
"70","71","72","73","74","75","76","77","78","79","7a","7b","7c","7d","7e","7f",
"80","81","82","83","84","85","86","87","88","89","8a","8b","8c","8d","8e","8f",
"90","91","92","93","94","95","96","97","98","99","9a","9b","9c","9d","9e","9f",
"a0","a1","a2","a3","a4","a5","a6","a7","a8","a9","aa","ab","ac","ad","ae","af",
"b0","b1","b2","b3","b4","b5","b6","b7","b8","b9","ba","bb","bc","bd","be","bf",
"c0","c1","c2","c3","c4","c5","c6","c7","c8","c9","ca","cb","cc","cd","ce","cf",
"d0","d1","d2","d3","d4","d5","d6","d7","d8","d9","da","db","dc","dd","de","df",
"e0","e1","e2","e3","e4","e5","e6","e7","e8","e9","ea","eb","ec","ed","ee","ef",
"f0","f1","f2","f3","f4","f5","f6","f7","f8","f9","fa","fb","fc","fd","fe","ff",
		};
#elif !defined(HAVE_PTSNAME)
static const
char	MasterDevice[] = "/dev/pty%s",
	SlaveDevice[] = "/dev/tty%s",
	*const deviceNo[] = {
		"p0","p1","p2","p3","p4","p5","p6","p7",
		"p8","p9","pa","pb","pc","pd","pe","pf",
		"q0","q1","q2","q3","q4","q5","q6","q7",
		"q8","q9","qa","qb","qc","qd","qe","qf",
		"r0","r1","r2","r3","r4","r5","r6","r7",
		"r8","r9","ra","rb","rc","rd","re","rf",
		"s0","s1","s2","s3","s4","s5","s6","s7",
		"s8","s9","sa","sb","sc","sd","se","sf",
		0,
	};
#endif
#endif /* !defined(HAVE_OPENPTY) */

#ifndef HAVE_SETEUID
# ifdef HAVE_SETREUID
#  define seteuid(e)	setreuid(-1, (e))
# else /* NOT HAVE_SETREUID */
#  ifdef HAVE_SETRESUID
#   define seteuid(e)	setresuid(-1, (e), -1)
#  else /* NOT HAVE_SETRESUID */
    /* I can't set euid. (;_;) */
#  endif /* HAVE_SETRESUID */
# endif /* HAVE_SETREUID */
#endif /* NO_SETEUID */

//rb_waitpid
//cpid = rb_waitpid(info->child_pid, &status, WUNTRACED);
//mruby//build/mrbgems/mruby-process/src/process.c
//static int mrb_waitpid(int pid, int flags, int *st);
static int
mrb_waitpid(int pid, int flags, int *st)
{
  int result;

retry:
  result = waitpid(pid, st, flags);
  if (result < 0) {
    if (errno == EINTR) {
      goto retry;
    }
    return -1;
  }

  return result;
}


#define VALUE mrb_value //struct RClass *

#define rb_pid_t int
#define rb_gid_t int
#define rb_uid_t int

#define Qnil mrb_nil_value()
#define Qtrue mrb_true_value()
#define Qfalse mrb_false_value()

VALUE rb_cFile;

typedef struct rb_io_t {
    int fd;                     /* file descriptor */
    FILE *stdio_file;           /* stdio ptr for read/write if available */
    int mode;                   /* mode flags: FMODE_XXXs */
    rb_pid_t pid;               /* child's pid (for pipes) */
    int lineno;                 /* number of lines read */
    VALUE pathv;                /* pathname for file */

    //void (*finalize)(struct rb_io_t*,int); /* finalize proc */
    //rb_io_buffer_t wbuf, rbuf;
    //VALUE tied_io_for_writing;

    /*
     * enc  enc2 read action                      write action
     * NULL NULL force_encoding(default_external) write the byte sequence of str
     * e1   NULL force_encoding(e1)               convert str.encoding to e1
     * e1   e2   convert from e2 to e1            convert str.encoding to e2
     */
    //struct rb_io_enc_t {
    //    rb_encoding *enc;
    //    rb_encoding *enc2;
    //    int ecflags;
    //    VALUE ecopts;
    //} encs;
    //rb_econv_t *readconv;
    //rb_io_buffer_t cbuf;
    //rb_econv_t *writeconv;
    //VALUE writeconv_asciicompat;
    //int writeconv_pre_ecflags;
    //VALUE writeconv_pre_ecopts;
    //int writeconv_initialized;
    //VALUE write_lock;
} rb_io_t;






















VALUE ruby_current_thread;

static inline VALUE
GET_THREAD(void) {
  return ruby_current_thread;
}

VALUE
rb_thread_current(void)
{
    return GET_THREAD();
}

typedef struct {
  int argc;
  mrb_value* argv;
  struct RProc* proc;
  pthread_t thread;
  mrb_state* mrb_caller;
  mrb_state* mrb;
  mrb_value result;
  mrb_bool alive;
} mrb_thread_context;

static void
mrb_thread_context_free(mrb_state *mrb, void *p) {
  if (p) {
    mrb_thread_context* context = (mrb_thread_context*) p;
    if (context->mrb && context->mrb != mrb) mrb_close(context->mrb);
    pthread_kill(context->thread, SIGINT);
    if (context->argv) free(context->argv);
    free(p);
  }
}

static const struct mrb_data_type mrb_thread_context_type = {
  "mrb_thread_context", mrb_thread_context_free,
};

//mruby//build/mrbgems/mruby-thread/src/mrb_thread.c
//static mrb_value mrb_thread_kill(mrb_state* mrb, mrb_value self);
static mrb_value
mrb_thread_kill(mrb_state* mrb, mrb_value self) {
  mrb_value value_context = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "context"));
  mrb_thread_context* context = NULL;
  Data_Get_Struct(mrb, value_context, &mrb_thread_context_type, context);
  if (context->mrb == NULL) {
    return mrb_nil_value();
  }
  pthread_kill(context->thread, SIGINT);
  mrb_close(context->mrb);
  context->mrb = NULL;
  return context->result;
}

static mrb_value mrb_obj_ivar_get(mrb_state *mrb, mrb_value self);


//static VALUE eChildExited;
static struct RClass *eChildExited;
//static VALUE cPTY;
static struct RClass *cPTY;
static mrb_state *mrb;

static VALUE
//echild_status(self)
//    VALUE self;
echild_status(mrb_state *mrb, VALUE self)
{
    return mrb_iv_get(mrb, self, mrb_intern_cstr(mrb, "status"));
}

struct pty_info {
    int fd;
    rb_pid_t child_pid;
    VALUE thread;
};

static void
raise_from_wait(state, info)
    struct pty_info *info;
    char *state;
{
/*
    extern VALUE rb_last_status;
    char buf[1024];
    VALUE exc;

    snprintf(buf, sizeof(buf), "pty - %s: %ld", state, (long)info->child_pid);
    exc = rb_exc_new2(eChildExited, buf);
    rb_iv_set(exc, "status", rb_last_status);
    rb_funcall(info->thread, rb_intern("raise"), 1, exc);
*/
}

static VALUE
pty_syswait(info)
    struct pty_info *info;
{
    int cpid, status;

    for (;;) {
	cpid = mrb_waitpid(info->child_pid, WUNTRACED, &status);
	if (cpid == -1) return Qnil;

#if defined(WIFSTOPPED)
#elif defined(IF_STOPPED)
#define WIFSTOPPED(status) IF_STOPPED(status)
#else
---->> Either IF_STOPPED or WIFSTOPPED is needed <<----
#endif /* WIFSTOPPED | IF_STOPPED */
	if (WIFSTOPPED(status)) { /* suspend */
	    raise_from_wait("stopped", info);
	}
	else if (kill(info->child_pid, 0) == 0) {
	    raise_from_wait("changed", info);
	}
	else {
	    raise_from_wait("exited", info);
	    return Qnil;
	}
    }
}

#define _(args) args

static void getDevice _((int*, int*, char [DEVICELEN]));

struct exec_info {
    int argc;
    VALUE *argv;
};

static VALUE pty_exec _((VALUE v));

static VALUE
pty_exec(v)
    VALUE v;
{
    //struct exec_info *arg = (struct exec_info *)v;
    //return rb_f_exec(arg->argc, arg->argv);
    return Qnil;
}

static void
establishShell(argc, argv, info, SlaveName)
    int argc;
    VALUE *argv;
    struct pty_info *info;
    char SlaveName[DEVICELEN];
{
    int 		i,master,slave;
    char		*p, tmp, *getenv();
    struct passwd	*pwent;
    VALUE		v;
    struct exec_info	arg;
    int			status;

    if (argc == 0) {
	char *shellname;

	if ((p = getenv("SHELL")) != NULL) {
	    shellname = p;
	}
	else {
	    pwent = getpwuid(getuid());
	    if (pwent && pwent->pw_shell)
		shellname = pwent->pw_shell;
	    else
		shellname = "/bin/sh";
	}
	//v = rb_str_new2(shellname);
	v = mrb_str_new_cstr(mrb, shellname);
	argc = 1;
	argv = &v;
    }
    getDevice(&master, &slave, SlaveName);

    info->thread = rb_thread_current();
    if((i = fork()) < 0) {
	close(master);
	close(slave);
	mrb_sys_fail(mrb, "fork failed");
    }

    if(i == 0) {	/* child */
	/*
	 * Set free from process group and controlling terminal
	 */
#ifdef HAVE_SETSID
	(void) setsid();
#else /* HAS_SETSID */
# ifdef HAVE_SETPGRP
#  ifdef SETGRP_VOID
	if (setpgrp() == -1)
	    perror("setpgrp()");
#  else /* SETGRP_VOID */
	if (setpgrp(0, getpid()) == -1)
	    mrb_sys_fail(mrb, "setpgrp()");
	if ((i = open("/dev/tty", O_RDONLY)) < 0)
	    mrb_sys_fail(mrb, "/dev/tty");
	else {
	    if (ioctl(i, TIOCNOTTY, (char *)0))
		perror("ioctl(TIOCNOTTY)");
	    close(i);
	}
#  endif /* SETGRP_VOID */
# endif /* HAVE_SETPGRP */
#endif /* HAS_SETSID */

	/*
	 * obtain new controlling terminal
	 */
#if defined(TIOCSCTTY)
	close(master);
	(void) ioctl(slave, TIOCSCTTY, (char *)0);
	/* errors ignored for sun */
#else
	close(slave);
	slave = open(SlaveName, O_RDWR);
	if (slave < 0) {
	    perror("open: pty slave");
	    _exit(1);
	}
	close(master);
#endif
	write(slave, "", 1);
	dup2(slave,0);
	dup2(slave,1);
	dup2(slave,2);
	close(slave);
#if defined(HAVE_SETEUID) || defined(HAVE_SETREUID) || defined(HAVE_SETRESUID)
	seteuid(getuid());
#endif

	arg.argc = argc;
	arg.argv = argv;
  //mrb_value v = mrb_funcall(mrb, ruby_res, mrb_intern(mrb, "to_int"), 0);
	//rb_protect(pty_exec, (VALUE)&arg, &status);

	sleep(1);
	_exit(1);
    }

    read(master, &tmp, 1);
    close(slave);

    info->child_pid = i;
    info->fd = master;
}

static VALUE
pty_finalize_syswait(info)
    struct pty_info *info;
{
    mrb_thread_kill(mrb, info->thread);
    //rb_funcall(info->thread, rb_intern("value"), 0);
    //rb_detach_process(info->child_pid);
    return Qnil;
}

static int
get_device_once(master, slave, SlaveName, fail)
    int *master, *slave, fail;
    char SlaveName[DEVICELEN];
{
#if defined HAVE_OPENPTY
/*
 * Use openpty(3) of 4.3BSD Reno and later,
 * or the same interface function.
 */
    if (openpty(master, slave, SlaveName,
		(struct termios *)0, (struct winsize *)0) == -1) {
	if (!fail) return -1;
	mrb_raise(mrb, E_RUNTIME_ERROR, "openpty() failed");
    }

    return 0;
#elif defined HAVE__GETPTY
    char *name;

    if (!(name = _getpty(master, O_RDWR, 0622, 0))) {
	if (!fail) return -1;
	mrb_raise(mrb, E_RUNTIME_ERROR, "_getpty() failed");
    }

    *slave = open(name, O_RDWR);
    strncpy(SlaveName, name, sizeof SlaveName);

    return 0;
#else /* HAVE__GETPTY */
    int	 i,j;

#ifdef HAVE_PTSNAME
    char *pn;
    void (*s)();

    extern char *ptsname(int);
    extern int unlockpt(int);
    extern int grantpt(int);

    if((i = open("/dev/ptmx", O_RDWR, 0)) != -1) {
	s = signal(SIGCHLD, SIG_DFL);
	if(grantpt(i) != -1) {
	    signal(SIGCHLD, s);
	    if(unlockpt(i) != -1) {
		if((pn = ptsname(i)) != NULL) {
		    if((j = open(pn, O_RDWR, 0)) != -1) {
#if defined I_PUSH && !defined linux
			if(ioctl(j, I_PUSH, "ptem") != -1) {
			    if(ioctl(j, I_PUSH, "ldterm") != -1) {
				ioctl(j, I_PUSH, "ttcompat");
#endif
				*master = i;
				*slave = j;
				strncpy(SlaveName, pn, sizeof SlaveName);
				return 0;
#if defined I_PUSH && !defined linux
			    }
			}
#endif
		    }
		}
	    }
	}
	close(i);
    }
    if (!fail) mrb_raise(mrb, E_RUNTIME_ERROR, "can't get Master/Slave device");
    return -1;
#else
    char **p;
    char MasterName[DEVICELEN];

    for (p = deviceNo; *p != NULL; p++) {
      snprintf(MasterName, sizeof MasterName, MasterDevice, *p);
      if ((i = open(MasterName,O_RDWR,0)) >= 0) {
        *master = i;
        snprintf(SlaveName, sizeof SlaveName, SlaveDevice, *p);
        if ((j = open(SlaveName,O_RDWR,0)) >= 0) {
          *slave = j;
          chown(SlaveName, getuid(), getgid());
          chmod(SlaveName, 0622);
          return 0;
        }
        close(i);
      }
    }
    if (fail) mrb_raisef(mrb, E_RUNTIME_ERROR, "can't get %s", SlaveName);
    return -1;
#endif
#endif
}

static void
getDevice(master, slave, slavename)
    int *master, *slave;
    char slavename[DEVICELEN];
{
    if (get_device_once(master, slave, slavename, 0)) {
	rb_gc();
	get_device_once(master, slave, slavename, 1);
    }
}

/*
#define MakeOpenFile(obj, fp) do {\
    if (RFILE(obj)->fptr) {\
        rb_io_close(obj);\
        rb_io_fptr_finalize(RFILE(obj)->fptr);\
        RFILE(obj)->fptr = 0;\
    }\
    (fp) = 0;\
    RB_IO_FPTR_NEW(fp);\
    RFILE(obj)->fptr = (fp);\
} while (0)
*/

/* ruby function: getpty */
static VALUE
pty_getpty(argc, argv, self)
    int argc;
    VALUE *argv;
    VALUE self;
{
    VALUE res;
/*
    struct pty_info info;
    struct pty_info thinfo;
    rb_io_t *wfptr,*rfptr;
    VALUE rport = rb_obj_alloc(rb_cFile);
    VALUE wport = rb_obj_alloc(rb_cFile);
    char SlaveName[DEVICELEN];

    MakeOpenFile(rport, rfptr);
    MakeOpenFile(wport, wfptr);

    establishShell(argc, argv, &info, SlaveName);

    rfptr->mode = rb_io_mode_flags("r");
    rfptr->f = fdopen(info.fd, "r");
    rfptr->path = strdup(SlaveName);

    wfptr->mode = rb_io_mode_flags("w") | FMODE_SYNC;
    wfptr->f = fdopen(dup(info.fd), "w");
    wfptr->path = strdup(SlaveName);

    res = rb_ary_new2(3);
    rb_ary_store(res,0,(VALUE)rport);
    rb_ary_store(res,1,(VALUE)wport);
    rb_ary_store(res,2,INT2FIX(info.child_pid));

    thinfo.thread = rb_thread_create(pty_syswait, (void*)&info);
    thinfo.child_pid = info.child_pid;
    rb_thread_schedule();

    if (rb_block_given_p()) {
	rb_ensure(rb_yield, res, pty_finalize_syswait, (VALUE)&thinfo);
	return Qnil;
    }
    return res;
*/

  return res;
}

/* ruby function: protect_signal - obsolete */
static VALUE
pty_protect(self)
    VALUE self;
{
    rb_warn("PTY::protect_signal is no longer needed");
    rb_yield(Qnil);
    return self;
}

/* ruby function: reset_signal - obsolete */
static VALUE
pty_reset_signal(self)
    VALUE self;
{
    rb_warn("PTY::reset_signal is no longer needed");
    return self;
}


void
Init_pty(mrb_state *mrb_)
{
    mrb = mrb_;

    cPTY = mrb_define_module(mrb, "PTY");
    //mrb_define_module_function(mrb, cPTY,"getpty",pty_getpty,-1);
    mrb_define_module_function(mrb, cPTY,"spawn",pty_getpty,-1);
    //mrb_define_module_function(mrb, cPTY,"protect_signal",pty_protect,0);
    //mrb_define_module_function(mrb, cPTY,"reset_signal",pty_reset_signal,0);

    eChildExited = mrb_define_class_under(mrb, cPTY,"ChildExited", E_RUNTIME_ERROR);
    mrb_define_method(mrb, eChildExited,"status",echild_status,MRB_ARGS_NONE());
}
