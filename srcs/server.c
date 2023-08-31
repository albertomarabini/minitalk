/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/29 14:21:23 by amarabin          #+#    #+#             */
/*   Updated: 2023/08/31 09:59:09 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minitalk.h"

/**
 * USED FUNCTIONS
 * -Server
 * sigaction
 *  Sets the action taken by a process on receipt of a specific signal.
 *  int sigaction(int signum, const struct sigaction *act,
	struct sigaction *oldact); (-1 on failure)
 *  Similar to signal, used for registering signal handlers. You can also
 *  specify flags to control behavior, such as restarting system calls, or
 *  which signals should be blocked during the execution of the handler.
 *
 *  Setting the sigaction struct sa_flags field as SA_SIGINFO allows to specify
 *  that the signal handler function will be of the sigaction type.
 *  thus that the signal handler is expected to match the following signature:
 *  void handler(int signum, siginfo_t *info, void *ucontext);
 *  that allows the handler to receive additional information about the signal.
 *  (eg: the pid of the client,the user ID -UID- of the sending user, etc.)
 *  In contrasts a "simple" signal handler, will ave the following signature:
 *  void simple_handler(int signum);
 *  that will allow only for the signal number to be passed to the handler
 *  just like signal.
 * pause
 *  Causes the calling process to sleep until a signal is received.
 *  int pause(void); Returns -1 and errno is set to EINTR when interrupted by
 *  a signal handler.
 *  You call pause in an infinite loop until the server receives a signal.
 * getpid
 *  Returns the process ID of the calling process.
 *  pid_t getpid(void);
 *  used to get the server process ID and display it.
 *
 * -Client
 * kill
 *  Sends a signal to a process or a group of processes.
 *  int kill(pid_t pid, int sig); (-1 on failure)
 *  we use kill to send signals from the client to the server.
 * usleep
 *  Makes the process sleep for a specified number of microseconds.
 *  int usleep(useconds_t usec); (-1 on failure)
 *  introduce small delays between sending individual bits to ensure the server
 *  has enough time to process them.
 *
 * NOT USED FUNCTIONS:
 * signal
 *  associates a signal (e.g., SIGUSR1, SIGUSR2) with a signal handler function
 *  sighandler_t signal(int signum, sighandler_t handler); (SIG_ERR on error)
 *  Register a function to handle incoming signals.
 *  signal(SIGUSR1, handler_function); sets handler_function to handle SIGUSR1.
 *  TOO SIMPLE:
 *  here is not used because thinking of the final acknoledgement from the
 *  server in the bonus, i needed to receive the PID of the client.
 * sigemptyset
 *  Initializes a signal set to be empty.
 *  int sigemptyset(sigset_t *set); (-1 on failure)
 *  Before using a sigset_t variable (which represents a set of signals),
 *  it needs to be initializedto an empty set.
 *  NOT NEEDED
 * sigaddset
 *  Adds a signal to an existing set of signals.
 *  int sigaddset(sigset_t *set, int signum); (-1 on failure)
 *  After initializing a signal set with sigemptyset, you add specific signals
 *  to it using sigaddset.
 *  NOT NEEDED
 * sleep
 *  Makes the process sleep for a specified number of seconds.
 *  unsigned int sleep(unsigned int seconds);
 *  TOO BROAD
 *
 * Probable causes of error:
 * Concurrency and Signal Coalescing: Signal handling isn't perfectly serial
 * most of the time, even if it appears that way. Multiple signals of the same
 * type that arrive in quick succession could be coalesced into a single signal,
 * causing you to miss bits.
 * Signal Ordering: Signals aren't guaranteed to be processed in the order they
 * are sent, which could scramble your bytes.
 */

/**
 * Why i need a global variable? At very least because i need to collect
 * the bits and store them somewhere before to come up with a coherent byte.
 */
static t_buffer	g_buffer;

void	reset_byte(void)
{
	g_buffer.current_byte = 0;
	g_buffer.current_bit = 7;
}

/**
 * SIGNAL HANDLER
 * This function hadles the signal received by the client.
 * the line
 * g_buffer.current_byte |= (1 << g_buffer.current_bit);
 * will mark a 1 in the byte buffer at the current bit position.
 * at byte completion we print it or, if it was a '\0',
 * we print a new line.
 */
void	handle_signal(int sig, siginfo_t *info, void *context)
{
	if (sig == SIGUSR1)
		g_buffer.current_byte |= (1 << g_buffer.current_bit);
	if (--g_buffer.current_bit < 0)
	{
		if (g_buffer.current_byte == '\0')
			ft_putchar_fd('\n', 1);
		else
			ft_putchar_fd(g_buffer.current_byte, 1);
		reset_byte();
	}
}

int	main(void)
{
	struct sigaction	sa;

	printf("Server PID: %d\n", getpid());
	reset_byte();
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handle_signal;
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	while (1)
		pause();
	return (0);
}
