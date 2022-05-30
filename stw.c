#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum {
	ret_suc = 0,
	ret_fl = 1,
	md_normal = 0,
	md_show = 1
};

const char *m_err = "error";
const char *flag_help = "--help";
const char *flag_show = "-s";

const char *m_help =
"	Usage: %s [-s] <file>\n"
"	Without \'-s\' flag, initial stopwatch time will be acquired from\n"
"	<file>.	If <file> doesn't exist, it will be created and initial\n"
"	stopwatch time will be zero. After that will be launched.\n"
"	With \'-s\' flag, time from <file> contents will be retieved and\n"
"	printed, but stopwatch won't start.\n";
const char *m_help_inv = "Ask for help properly!";
const char *m_mode_dup = "mode already specified";
const char *m_file_dup = "file already specified";
const char *m_fmiss = "missing input file";
const char *m_fexist = "checking existence";
const char *m_fnot_exist = "file doesn't exist";
const char *m_inv_fmt = "invalid format";
const char *m_hsfx = " hours, ";
const char *m_msfx = " min, ";
const char *m_ssfx = ".";
const char *m_mssfx = " sec";
const char *m_numh = "hours number should be non-negative";
const char *m_numm = "minutes number should be in [0; 59] interval";
const char *m_nums = "seconds number should be in [0; 59]; interval";
const char *m_numms =
		"milliseconds number should be in [0; 999] interval";

const int user_sig = SIGINT;
struct timeval tvstart;
struct timeval tvend;

struct info {
	int mode;
	char *file;

	int prevsec;
	int prevmsec;
};

static void die(char *fmt, ...);


static void arg_parse(int argc, char **argv, struct info *ints);
static void operate(struct info *ints);
	
int main(int argc, char **argv)
{
	struct info ints;
	memset(&ints, 0, sizeof(ints));
	arg_parse(argc, argv, &ints);
	operate(&ints);
	return ret_suc;
}

static char **arg_parse_opts(char **argv, struct info *ints);
static void arg_parse_other(char **argv, struct info *ints);
static void info_check(struct info *ints);

static void arg_parse(int argc, char **argv, struct info *ints)
{
	if (argc == 1) {
		printf(m_help, argv[0]);
		exit(ret_suc);
	}
	argv = arg_parse_opts(argv + 1, ints);
	arg_parse_other(argv, ints);
	info_check(ints);
}

static char **arg_parse_opts(char **argv, struct info *ints)
{
	for (; *argv; ++argv) {
		if (strcmp(*argv, flag_help) == 0) {
			if (argv[1] != NULL)
				die("\t%s\n", m_help_inv);
			printf(m_help, argv[-1]);
			exit(ret_suc);
		} else if (strcmp(*argv, flag_show) == 0) {
			if (ints->mode == md_show)
				die("%s: %s: %s\n", m_err, *argv, m_mode_dup);
			ints->mode = md_show;
		} else
			return argv;
	}
	return argv;
}

static void arg_parse_other(char **argv, struct info *ints)
{
	for (; *argv; ++argv) {
		if (ints->file != NULL)
			die("%s: %s: %s\n", m_err, *argv, m_file_dup);
		ints->file = *argv;
	}
}

static void info_check(struct info *ints)
{
	if (ints->file == NULL)
		die("%s: %s\n", m_err, m_fmiss);
}

static void operate_show(struct info *ints);
static void operate_normal(struct info *ints);

static void operate(struct info *ints)
{
	if (ints->mode == md_show)
		operate_show(ints);
	else
		operate_normal(ints);
}

static int file_exist(char *fname);
static void fread_secs(char *fname, int *h, int *m, int *s, int *ms);
static void write_secs(int h, int m, int s, int ms);

static void operate_show(struct info *ints)
{
	if (file_exist(ints->file) == 0) {
		die("%s: %s: %s\n", m_err, ints->file, m_fnot_exist);
	} else {
		int h, m, s;
		fread_secs(ints->file, &h, &m, &s, &ints->prevmsec);
		write_secs(h, m, s, ints->prevmsec);
	}
}

static int file_exist(char *fname)
{
	if (access(fname, F_OK) == -1) {
		if (errno == ENOENT)
			return 0;
		die("%s: %s: %s\n", m_err, m_fexist, strerror(errno));
	}
	return 1;
}

static void check(int h, int m, int s, int ms);

static void fread_secs(char *fname, int *h, int *m, int *s, int *ms)
{
	 FILE *f = fopen(fname, "r");
	 if (f == NULL)
		 die("%s: %s: %s\n", m_err, fname, strerror(errno));
	 if (fscanf(f, "%d %d%d %d\n", h, m, s, ms) != 4)
		 die("%s: %s: %s\n", m_err, fname, m_inv_fmt);
	 if (fgetc(f) != EOF)
		 die("%s: %s: %s\n", m_err, fname, m_inv_fmt);
	 check(*h, *m, *s, *ms);
	 fclose(f);
}

static void check(int h, int m, int s, int ms)
{
	if (h < 0)
		die("%s: %s\n", m_err, m_numh);
	if (m < 0)
		die("%s: %s\n", m_err, m_numm);
	if (s < 0)
		die("%s: %s\n", m_err, m_nums);
	if (ms < 0 || ms > 999)
		die("%s: %s\n", m_err, m_numms);
}

static void write_secs(int h, int m, int s, int ms)
{
	printf("%d%s%d%s%d%s%d%s\n", h, m_hsfx, m, m_msfx, s, m_ssfx,
			ms, m_mssfx);
}

static void get_time(struct timeval *tv);
static void signalblock();
static void sigsetup();
static void get_secs(struct info *ints);
static void sigallow();
static void calc_new(struct info *ints);
static void write_new(struct info *ints);

static void operate_normal(struct info *ints)
{
	get_time(&tvstart);
	signalblock();
	sigsetup();
	get_secs(ints);
	sigallow();
	calc_new(ints);
	write_new(ints);
}

static void get_time(struct timeval *tv)
{
	gettimeofday(tv, NULL);
	tv->tv_usec /= 1000;	/* milliseconds precision */
}

static void signalblock()
{
	sigset_t ss;
	sigemptyset(&ss);
	sigaddset(&ss, user_sig);
	sigprocmask(SIG_BLOCK, &ss, NULL);
}

static void sighandler(int v);

static void sigsetup()
{
	sigset_t ss;
	struct sigaction act;
	sigemptyset(&ss);
	act.sa_handler = sighandler;
	act.sa_mask = ss;
	act.sa_flags = 0;
	sigaction(user_sig, &act, NULL);
}

static void get_time(struct timeval *tv);
static void sighandler(int v)
{
	get_time(&tvend);
}

static int file_exist(char *fname);
static void fread_secs(char *fname, int *h, int *m, int *s, int *ms);
static int convert(int h, int m, int s);

static void get_secs(struct info *ints)
{
	int h, m, s;
	if (file_exist(ints->file) == 1) {
		fread_secs(ints->file, &h, &m, &s, &ints->prevmsec);
		ints->prevsec = convert(h, m, s);
	}
}

static int convert(int h, int m, int s)
{
	return s + 60 * (m + 60 * h);
}

static void sigallow()
{
	sigset_t ss;
	sigprocmask(SIG_SETMASK, NULL, &ss);
	sigdelset(&ss, user_sig);
	sigsuspend(&ss);
}

static void calc_new(struct info *ints)
{
	ints->prevsec += tvend.tv_sec - tvstart.tv_sec;
	ints->prevmsec += tvend.tv_usec - tvstart.tv_usec;
	if (ints->prevmsec < 0) {
		ints->prevsec--;
		ints->prevmsec += 1000;
	}
	if (ints->prevmsec > 999) {
		ints->prevsec++;
		ints->prevmsec -= 1000;
	}
}

static void convert2(int sec, int *h, int *m, int *s);
static void fwrite_secs(char *fname, int h, int m, int s, int ms);

static void write_new(struct info *ints)
{
	int h, m, s;
	convert2(ints->prevsec, &h, &m, &s);
	fwrite_secs(ints->file, h, m, s, ints->prevmsec);
}

static void convert2(int sec, int *h, int *m, int *s)
{
	*h = sec / 3600;
	*m = sec % 3600 / 60;
	*s = sec % 60;
}

static void fwrite_secs(char *fname, int h, int m, int s, int ms)
{
	FILE *f = fopen(fname, "w");
	if (f == NULL)
		die("%s: %s: %s\n", m_err, fname, strerror(errno));
	fprintf(f, "%d %d %d %d\n", h, m, s, ms);
	fclose(f);
}

static void die(char *fmt, ...)
{
	va_list vl;
	va_start(vl, fmt);
	vfprintf(stderr, fmt, vl);
	va_end(vl);
	exit(ret_fl);
}
