use ps -ef to check the threads.
ps -ef 
[mythread] - shown in closed brackets

ps -f -p 2 --ppid 2 if you want to see kthreads only.

by default kernel thread ignores all the signals, so we need to call allow_signal()
so that kernel obeys the signal.
allow_signal(SIG_KILL); - killall -9 thread will work.
