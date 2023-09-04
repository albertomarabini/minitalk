/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/29 14:21:23 by amarabin          #+#    #+#             */
/*   Updated: 2023/09/04 14:29:19 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minitalk_bonus.h"

static int	g_ack;

void	print_byte(char c)
{
	for (int i = 7; i >= 0; i--)
	{
		printf("%d", (c >> i) & 1);
	}
	printf("\n");
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
	int					ack_count;
	int attmpt;
	unsigned long long	timer;
	unsigned long long	ack_total_wait_mcsc;

	ack_count = 0;
	timer = 0;
	attmpt = 0;
	// means Transaction Timeout in micrsrosec
	//	* 2 (since we are transmitting at half the speed)
	//	* length of the reset sequence expected by the server
	//
	//	* the number of times i'm potentially going to retry to send the reset sequence
	ack_total_wait_mcsc = TRANS_TIMEOUT_TMS * 100 * RESET_SEQUENCE_COUNT
		* RESET_SEQUENCE_ATTEMPTS;
	while (ack_count < REQUIRED_RESET_SERVER_ACKS
		&& timer < ack_total_wait_mcsc)
	{
		kill(srvr_pid, SIGUSR1); // Transmit a 1 bit
		attmpt++;
		usleep(TRANS_RATE_MCS);
		if (g_ack == 1)
			ack_count++;
		/*else if (g_ack == 0 && (attmpt + 1) % RESET_SEQUENCE_COUNT == 0)
				ack_count = 0; // Reset if not acknowledged*/
		timer += TRANS_RATE_MCS;
		g_ack = -1;
		if((attmpt + 1) % RESET_SEQUENCE_COUNT == 0)
			ft_printf("%CReset has been required: ack:%i att:%i\n", 'g', ack_count,
				timer);
	}
	if (ack_count < REQUIRED_RESET_SERVER_ACKS && timer > ack_total_wait_mcsc)
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
	int					max_reattempts;

	g_ack = -1;
	max_reattempts = 0;
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
		while (g_ack == -1 && timer < TRANS_TIMEOUT_TMS * 2)
		{
			usleep(100); // 1/10 mcs
			timer++;
		}
		if (i < len - 1)
			ft_printf("ack:%c %i nack:%i\n", argv[2][i], g_ack, nack);
		else
			ft_printf("EOF ack:%i", g_ack);
		if (i == len - 1 && g_ack >= 0)
		{
			if (g_ack == 1)
				ft_putstr_fd("Ack Transmission Success\n",1);
			else
				ft_putstr_fd("Ack Transmission Not Success\n",1);
			break ;
		}
		else if (g_ack == 1)
		{
			i++;
			nack = 0;
			max_reattempts = 0;
		}
		else if (g_ack == 0)
		{
			if (i < len - 1)
				print_byte(argv[2][i]);
			nack++;
			g_ack = -1;
			timer = 0;
			//we give enough time to the server to purge the buffer
			while (g_ack == -1 && TRANS_TIMEOUT_TMS * 8 > timer++)
				usleep(100); // 1/10 mcs
			//at this point the server should have sent us the reckon for restating transmissions
			if (g_ack == 1)
				ft_putstr_fd("Byte Resend flag up\n",1);
			else
				ft_putstr_fd("Cross your fngers\n",1);
			timer = 0;
			while (TRANS_TIMEOUT_TMS > timer++)
				usleep(100); // 1/10 mcs
		}
		if (nack > MAX_NACK) // Timeout or MAX_NACK
		{
			ft_putstr_fd("Timeout or Catastrophic failure\n",1);
			max_reattempts++;
			if (max_reattempts > MAX_REATTEMPTS)
			{
				ft_putstr_fd("Max Reattempts reached\n",1);
				break ;
			}
			if (!send_reset_sequence(srvr_pid))
			{
				ft_putstr_fd("Transmission Failure\n",1);
				break ;
			}
			nack = 0;
			i = 0;
		}
	}
	return (0);
}
