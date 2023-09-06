/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/29 14:21:23 by amarabin          #+#    #+#             */
/*   Updated: 2023/08/31 09:17:19 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../minitalk.h"

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
		usleep(300);
	}
}

int	main(int argc, char *argv[])
{
	int	srvr_pid;
	int	sig;
	int	i;
	int	len;
	int	bit_i;

	if (argc != 3)
	{
		ft_printf("Error\nIncorrect number of parameters");
		return (1);
	}
	srvr_pid = ft_atoi(argv[1]);
	i = 0;
	len = ft_strlen(argv[2]);
	while (len > i)
		transmit_char(argv[2][i++], srvr_pid);
	transmit_char('\0', srvr_pid);
	return (0);
}
