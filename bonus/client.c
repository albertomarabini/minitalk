/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/29 14:21:23 by amarabin          #+#    #+#             */
/*   Updated: 2023/09/06 18:54:19 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minitalk_bonus.h"

static int	g_ack;

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

void	handle_ack(int sig)
{
	if (sig == SIGUSR1)
		g_ack = 1;
	else if (sig == SIGUSR2)
		g_ack = 0;
}

void	transmit_char(char c, int srvr_pid)
{
	int	bit_i;
	int	sig;
	int	wait;

	bit_i = 8;
	while (0 < bit_i--)
	{
		sig = SIGUSR2;
		if ((c >> bit_i) & 1)
			sig = SIGUSR1;
		kill(srvr_pid, sig);
		usleep(TRANS_RATE_MCS);
	}
}

int	send_reset_sequence(int srvr_pid)
{
	int	ack_count;
	int	timer;
	int	ack_total_wait;

	ack_count = 0;
	timer = 0;
	ack_total_wait = 2 * RESET_SEQUENCE_COUNT * RESET_SEQUENCE_ATTEMPTS;
	while (ack_count < REQUIRED_RESET_SERVER_ACKS && timer < ack_total_wait)
	{
		kill(srvr_pid, SIGUSR1);
		usleep(TRANS_RATE_MCS * 2);
		if (g_ack == 1)
			ack_count++;
		timer++;
		g_ack = -1;
		if (timer % RESET_SEQUENCE_COUNT == 0)
			ft_printf("%CReset required. srv aks:%i att:%i\n", ack_count,
				timer);
	}
	if (ack_count < REQUIRED_RESET_SERVER_ACKS && timer >= ack_total_wait)
		return (0);
	return (1);
}

int	main(int argc, char *argv[])
{
	int					srvr_pid;
	int					i;
	int					len;
	unsigned long long	timer;
	int					nack;

	g_ack = -1;
	if (argc != 3 || ft_strlen(argv[2]) == 0)
	{
		ft_printf("Error\nIncorrect number of parameters");
		return (1);
	}
	srvr_pid = ft_atoi(argv[1]);
	signal(SIGUSR1, handle_ack);
	signal(SIGUSR2, handle_ack);
	i = 0;
	nack = 0;
	len = ft_strlen(argv[2]) + 1;
	while (len > i)
	{
		g_ack = -1;
		if (i < len - 1)
			transmit_char(argv[2][i], srvr_pid);
		else
			transmit_char('\0', srvr_pid);
		timer = 0;
		// waiting for acknowledgement
		while (g_ack == -1 && timer < TRANS_TIMEOUT_MCS / 100)
		{
			usleep(100); // 1/10 mcs
			timer++;
		}
		// if (i < len - 1)
		// 	ft_printf("ack:%c %i nack:%i\n", argv[2][i], g_ack, nack);
		// else
		// 	ft_printf("EOF ack:%i", g_ack);
		if (i == len - 1 && g_ack >= 0)
		{
			if (g_ack == 1)
				ft_putstr_fd("Ack Transmission Success\n", 1);
			else
				ft_putstr_fd("Ack Transmission Not Success\n", 1);
			break ;
		}
		else if (g_ack == 1)
		{
			i++;
			nack = 0;
		}
		else if (nack > MAX_NACK || g_ack == -1)
		{
			ft_putstr_fd("Timeout or trasmission failure\n", 1);
			if (nack > MAX_NACK || !send_reset_sequence(srvr_pid))
			{
				ft_putstr_fd("Catastrophic Failure\n", 1);
				break ;
			}
			i = 0;
		}
		else if (g_ack == 0)
		{
			// if (i < len - 1)
			// 	print_byte(argv[2][i]);
			nack++;
			g_ack = -1;
			timer = 0;
			// we give enough time to the server to purge the buffer
			while (g_ack == -1 && TRANS_TIMEOUT_MCS / 100 * 3 > timer++)
				usleep(100); // 1/10 mcs
			// at this point the server should have sent us the reckon for restating transmissions
			if (g_ack == 1)
				ft_putstr_fd("Server ready for retransmission\n", 1);
			else
				ft_putstr_fd("Cross your fngers\n", 1);
			// rollback to the last valid UTF-8 header or the current character if header
			if (!ft_wclenb(argv[2][i]))
			{
				while (i >= 0 && !ft_wclenb(argv[2][i]))
					i--;
			}
			// wait some more before to restart the transmission
			timer = 0;
			while (TRANS_TIMEOUT_MCS / 100 > timer++)
				usleep(100); // 1/10 mcs
		}
	}
	return (0);
}
