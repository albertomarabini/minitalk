/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/29 14:21:23 by amarabin          #+#    #+#             */
/*   Updated: 2023/09/04 14:36:40 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minitalk_bonus.h"

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
 */

static t_buffer	g_buffer;

void print_byte(char c) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (c >> i) & 1);
    }
    printf("\n");
}
void	reset_buffer(void)
{
	if (g_buffer.buffr != NULL)
		free(g_buffer.buffr);
	g_buffer.buffr = NULL;
	g_buffer.size = 0;
	g_buffer.len = 0;
	g_buffer.reset_req_sequence = 0;
	g_buffer.inactvty_cnt_msc = 0;
	g_buffer.expctd_bytes = 0;
	g_buffer.purge_dirty = 0;
}

void	reset_byte(void)
{
	g_buffer.currnt_byte = 0;
	g_buffer.currnt_bit = 7;
}

void	extend_buffer(void)
{
	char	*tmp;

	g_buffer.size += BUFFER_SIZE_INCREMENT;
	tmp = (char *)malloc((g_buffer.size + 1) * sizeof(char));
	if (tmp == NULL)
	{
		reset_buffer();
		exit(1);
	}
	if (g_buffer.buffr != NULL)
	{
		ft_memcpy(tmp, g_buffer.buffr, g_buffer.len);
		free(g_buffer.buffr);
	}
	g_buffer.buffr = tmp;
}

int	ft_wclenb(unsigned char c)
{
	if ((c & 0x80) == 0)
		return (1);
	else if ((c & 0xE0) == 0xC0)
		return (2);
	else if ((c & 0xF0) == 0xE0)
		return (3);
	else if ((c & 0xF8) == 0xF0)
		return (4);
	return (0);
}

/**
 * SIGNAL HANDLER
 * This function hadles the signal received by the client.
 * the line
 * g_buffer.currnt_byte |= (1 << g_buffer.currnt_bit);
 * will mark a 1 in the byte buffer at the current bit position.
 * at byte completion we check if we need to extend the buffer,
 * we attach the new byte and if it was a '\0' we print the buffer
 */
void	handle_signal(int sig, siginfo_t *info, void *context)
{
	unsigned char shift_hack;
	g_buffer.inactvty_cnt_msc = 0;
	if(g_buffer.purge_dirty == 1)
		return ;
	if (sig == SIGUSR1)
	{
		g_buffer.currnt_byte |= (1 << g_buffer.currnt_bit);
		g_buffer.reset_req_sequence++;
	}
	else
		g_buffer.reset_req_sequence = 0;
	if (g_buffer.reset_req_sequence == RESET_SEQUENCE_COUNT)
	{
		reset_buffer();
		reset_byte();
		g_buffer.reset_req_sequence = 0; // Reset the counter
		kill(info->si_pid, SIGUSR1);   // Acknowledge the reset
		ft_putstr_fd("Reset Acknowledged\n",1);
		return ;
	}
	if (--g_buffer.currnt_bit < 0)
	{
		//if a reset byte is perceived gets discarded
		if(g_buffer.currnt_byte == 0xFF){
			reset_byte();
			return ;
		}
		//Is EOF
		if (g_buffer.currnt_byte == '\0' && g_buffer.expctd_bytes == 0)
		{
			// Acknowledge the EOF
			kill(info->si_pid, SIGUSR1);
			ft_printf("Result: %s\n", g_buffer.buffr);
			ft_putstr_fd("Transmission Success\n",1);
			g_buffer.cmpltd_client_pid = info->si_pid;
			reset_buffer();
			reset_byte();
			return ;
		}
		//Trying to correct the shift bug
		// if ((g_buffer.expctd_bytes == 0 && !ft_wclenb(g_buffer.currnt_byte))
		// || (g_buffer.expctd_bytes > 0 && (g_buffer.currnt_byte >> 6) != 2))
		// {
		// 	ft_putstr_fd("shift hack\n",1);
		// 	print_byte(g_buffer.currnt_byte);
		// 	shift_hack = (g_buffer.currnt_byte << 1) | (g_buffer.currnt_byte >> 7);
		// 	print_byte(shift_hack);
		// 	if ((g_buffer.expctd_bytes == 0 && ft_wclenb(shift_hack))
		// 	|| (g_buffer.expctd_bytes > 0 && (shift_hack >> 6) == 2)){
		// 		g_buffer.currnt_byte = shift_hack;
		// 		ft_putstr_fd("shift hack go\n",1);
		// 	}
		// }

		//failures
		//is not a header byte or
		//is not an intermediate byte
		//ia a null byte but i was expecting something else
		if ((g_buffer.expctd_bytes == 0 && !ft_wclenb(g_buffer.currnt_byte))
			|| (g_buffer.expctd_bytes > 0 && (g_buffer.currnt_byte >> 6) != 2)
			|| (g_buffer.expctd_bytes > 0 && g_buffer.currnt_byte == '\0'))
		{
			print_byte(g_buffer.currnt_byte);
			// Acknowledge the invalid byte
			kill(info->si_pid, SIGUSR2);
			// ft_putstr_fd("Invalid Byte Detected\n",1);
			//print_byte(g_buffer.currnt_byte);
			reset_byte();
			g_buffer.purge_dirty= 1;
			int timer = 0;
			//we wait to have enough time to purge the buffer from eventual dirty bits
			while(timer < TRANS_TIMEOUT_TMS * 4)
			{
				usleep(100); // 1/10 mcs
				timer++;
			}
			//we reset the byte again
			reset_byte();
			g_buffer.purge_dirty= 0;
			//we tell the client to restart sending
			kill(info->si_pid, SIGUSR1);
			return ;
		}
		//is a valid header
		if (g_buffer.expctd_bytes == 0)
			g_buffer.expctd_bytes = ft_wclenb(g_buffer.currnt_byte);
		//at this point we have a valid byte
		// We acknowledge the valid byte
		kill(info->si_pid, SIGUSR1);
		//if the buffer is too short we extend it
		if (g_buffer.buffr == NULL || g_buffer.size == g_buffer.len)
			extend_buffer();
		//we attach the new byte
		g_buffer.buffr[g_buffer.len++] = g_buffer.currnt_byte;
		//we null the next one
		g_buffer.buffr[g_buffer.len] = '\0';
		//we adjust the number of expected bytes
		g_buffer.expctd_bytes--;
		//we reset the byte
		reset_byte();
	}
}

int	main(void)
{
	struct sigaction	sa;

	printf("Server PID: %d\n", getpid());
	reset_byte();
	reset_buffer();
	//g_buffer.semaphore = 0;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handle_signal;
	g_buffer.expctd_bytes = 0;
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
    while (1)
    {
        // check for inactivity
		if(g_buffer.inactvty_cnt_msc++ > CLNT_INACTIVITY_TMS)
		{
			reset_byte();
			reset_buffer();
			ft_putstr_fd("-",1);
		}
        usleep(100);  // sleep 1/10 mcs
    }
	return (0);
}

// void	handle_signal(int sig, siginfo_t *info, void *context)
// {
// 	if (sig == SIGUSR1)
// 		g_buffer.currnt_byte |= (1 << g_buffer.currnt_bit);
// 	if (--g_buffer.currnt_bit < 0)
// 	{
// 		if (g_buffer.buffr == NULL || g_buffer.size == g_buffer.len)
// 			extend_buffer();
// 		g_buffer.buffr[g_buffer.len] = g_buffer.currnt_byte;
// 		if (g_buffer.currnt_byte == '\0')
// 		{
// 			ft_printf("%s\n", g_buffer.buffr);
// 			// kill(info->si_pid, SIGUSR1);
// 			reset_buffer();
// 		}
// 		reset_byte();
// 	}
// }
