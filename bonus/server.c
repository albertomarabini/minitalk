/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/29 14:21:23 by amarabin          #+#    #+#             */
/*   Updated: 2023/09/06 19:06:10 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minitalk_bonus.h"

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

void	print_byte(unsigned char c)
{
	char	bit_str[9];
	int		i;

	i = 7;
	while (i >= 0)
	{
		bit_str[7 - i] = ((c >> i) & 1) + '0';
		i--;
	}
	bit_str[8] = '\n';
	write(1, bit_str, 9);
}
void	reset_msg_buffer(void)
{
	if (g_buffer.buffr != NULL)
		free(g_buffer.buffr);
	g_buffer.buffr = NULL;
	g_buffer.size = 0;
	g_buffer.len = 0;
	g_buffer.reset_req_sequence = 0;
	g_buffer.inactvty_cnt_msc = 0;
	g_buffer.expctd_bytes = 0;
	g_buffer.mutex = 0;
}

void	reset_byte(void)
{
	g_buffer.signal_buffer = 0;
	g_buffer.bit_count = 0;
}

void	extend_buffer(void)
{
	char	*tmp;

	g_buffer.size += BUFFER_SIZE_INCREMENT;
	tmp = (char *)malloc((g_buffer.size + 1) * sizeof(char));
	if (tmp == NULL)
	{
		reset_msg_buffer();
		exit(1);
	}
	if (g_buffer.buffr != NULL)
	{
		ft_memcpy(tmp, g_buffer.buffr, g_buffer.len);
		free(g_buffer.buffr);
	}
	g_buffer.buffr = tmp;
}

/**
 * Determines the number of bytes in a UTF-8 character based on its first byte.
 * The length of an UTF-8 character is determined by the the bits in the first
 * byte following this schema:
 * U+0000   U+007F   -> 0xxxxxxx                            -> 0-127
 *                mask: 10000000 (or 0x80 in hex) -> 0x80 & c == 0
 * U+0080   U+07FF   -> 110xxxxx 10xxxxxx                   -> 128-2047
 *                mask: 11100000 (or 0xE0 in hex) -> 0xE0 & c == 0xC0
 * U+0800   U+FFFF   -> 1110xxxx 10xxxxxx 10xxxxxx          -> 2048-65535
 *                mask: 11110000 (or 0xF0 in hex) -> 0xF0 & c == 0xE0
 * U+10000  U+10FFFF -> 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx -> 65536-1114111
 *                mask: 11111000 (or 0xF8 in hex) -> 0xF8 & c == 0xF0
 *
 * @param c The first byte of the UTF-8 character.
 * @return The total number of bytes the UTF-8 character.
 *         Returns 0 if c does not fit any of the UTF-8 encoding schemas.
 */
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
 * g_buffer.signal_buffer |= (1 << g_buffer.bit_count);
 * will mark a 1 in the byte buffer at the current bit position.
 * at byte completion we check if we need to extend the buffer,
 * we attach the new byte and if it was a '\0' we print the buffer
 */
void	handle_signal(int sig, siginfo_t *info, void *context)
{
	//g_buffer.mutex = 1
	g_buffer.inactvty_cnt_msc = 0;
	g_buffer.client_pid = info->si_pid;
	if (sig == SIGUSR1)
	{
		g_buffer.signal_buffer |= (1U << sizeof(g_buffer.signal_buffer) * 8 - 1
			- g_buffer.bit_count);
		g_buffer.reset_req_sequence++;
	}
	else
		g_buffer.reset_req_sequence = 0;
	if (g_buffer.reset_req_sequence == RESET_SEQUENCE_COUNT)
	{
		reset_msg_buffer();
		reset_byte();
		g_buffer.reset_req_sequence = 0; // Reset the counter
		kill(info->si_pid, SIGUSR1);     // Acknowledge the reset
		ft_putstr_fd("Reset Acknowledged\n", 1);
	}
	g_buffer.bit_count++;//here do i risk a disaster? I sjouldn't: the client is expecting acks at each byte, the only problem might be a reset sequence piling up with a byte transmission...
	//g_buffer.mutex = 0;
}

void	process_signal(int main_timer)
{
	int	timer;
	unsigned char current_byte;

	if (g_buffer.bit_count >= 8)
	{
		current_byte = (unsigned char)(g_buffer.signal_buffer >> 8);
		g_buffer.signal_buffer <<= 8;
		g_buffer.bit_count -= 8;

		//ft_printf("New Character Expected Bytes: %i\n", g_buffer.expctd_bytes);

		// if a reset byte (as part of the sequence) is perceived gets discarded
		if (current_byte == 0xFF && g_buffer.reset_req_sequence >= 8)
			return ;
		// Is a correct EOF
		if (current_byte == '\0' && g_buffer.expctd_bytes == 0)
		{
			// Acknowledge the EOF
			kill(g_buffer.client_pid, SIGUSR1);
			ft_printf("\nResult: %s\n", g_buffer.buffr);
			ft_putstr_fd("Transmission Success\n", 1);
			g_buffer.cmpltd_client_pid = g_buffer.client_pid;//?
			reset_msg_buffer();
			reset_byte();
			return ;
		}
		// failures
		// is not a header byte or
		// is not an intermediate byte
		// ia a null byte but i was expecting something else
		// maybe there is should be something like "if you didn't sent me 8 bit
		// and you are going into inativity, i will ask you to retransmit again"
		if ((g_buffer.expctd_bytes == 0 && !ft_wclenb(current_byte))
			|| (g_buffer.expctd_bytes > 0 && (current_byte >> 6) != 2)
			|| (g_buffer.expctd_bytes > 0 && current_byte == '\0'))
		{
			//g_buffer.mutex = 1;
			// if ((g_buffer.expctd_bytes == 0 && !ft_wclenb(current_byte)))
			// 	ft_putstr_fd("Invalid 1",1);
			// if (g_buffer.expctd_bytes > 0 && (current_byte >> 6) != 2)
			// 	ft_putstr_fd("Invalid 2",1);
			// if (g_buffer.expctd_bytes > 0 && current_byte == '\0')
			// 	ft_putstr_fd("Invalid 3",1);
			// ft_printf(" Expected Bytes: %i\n", g_buffer.expctd_bytes);
			ft_putstr_fd("Invalid ",1);
			print_byte(current_byte);
			// Acknowledge the invalid byte
			kill(g_buffer.client_pid, SIGUSR2);
			// ft_putstr_fd("Invalid Byte Detected\n",1);
			// print_byte(current_byte);
			//g_buffer.mutex = 1;
			timer = 0;
			while (timer < TRANS_TIMEOUT_MCS / 100)
			{
				usleep(100); // 1/10 mcs
				timer++;
			}
			// we reset the byte again
			reset_byte();
			//we roll back the buffer
			if(g_buffer.expctd_bytes > 0)
			{
				while(g_buffer.len >= 0 && !ft_wclenb(g_buffer.buffr[--g_buffer.len]))
					g_buffer.buffr[g_buffer.len] = '\0';
				g_buffer.buffr[g_buffer.len] = '\0';
				g_buffer.expctd_bytes = 0;
			}
			//g_buffer.mutex = 0;
			// we tell the client to restart sending
			kill(g_buffer.client_pid, SIGUSR1);
			return ;
		}
		// is a valid header we update the number of expected bytes
		//ft_putstr_fd("Good character\n",1);
		//ft_printf("Good Character Expected Bytes: %i\n", g_buffer.expctd_bytes);
		if (g_buffer.expctd_bytes == 0)
			g_buffer.expctd_bytes = ft_wclenb(current_byte);
		// at this point we have a valid byte
		// if the result buffer is too short we extend it
		if (g_buffer.buffr == NULL || g_buffer.size == g_buffer.len)
			extend_buffer();
		// we attach the new byte at the end of the buffer
		g_buffer.buffr[g_buffer.len++] = current_byte;
		// we null the next one
		g_buffer.buffr[g_buffer.len] = '\0';
		// we adjust the number of expected bytes
		g_buffer.expctd_bytes--;
		// We acknowledge the valid byte
		kill(g_buffer.client_pid, SIGUSR1);
	}
}

// while (1)
// {
//     // Poll to check if a signal was received
//     process_signal();

int	main(void)
{
	struct sigaction	sa;
	int main_timer;

	printf("Server PID: %d\n", getpid());
	reset_byte();
	reset_msg_buffer();
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handle_signal;
	g_buffer.expctd_bytes = 0;
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	main_timer = 0;
	while (1)
	{
		process_signal(main_timer++);
		// check for inactivity
		if (CLNT_INACTIVITY_MCS / 100 < g_buffer.inactvty_cnt_msc++)
		{
			reset_byte();
			reset_msg_buffer();
			ft_putstr_fd("-", 1);
		}
		usleep(100); // sleep 1/10 mcs
	}
	return (0);
}
