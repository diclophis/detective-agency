#
# Regular cron jobs for the detective-agency package
#
0 4	* * *	root	[ -x /usr/bin/detective-agency_maintenance ] && /usr/bin/detective-agency_maintenance
