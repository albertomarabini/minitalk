/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minitalk_bonus.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/02/21 20:15:54 by prossi            #+#    #+#             */
/*   Updated: 2023/09/04 14:39:52 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINITALK_BONUS_H
# define MINITALK_BONUS_H

# include "../ft_printf/ft_printf.h"
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

# define MAX_NACK 10
# define MAX_REATTEMPTS 10
# define TRANS_RATE_MCS 800ULL    // 100000ULL
# define TRANS_TIMEOUT_TMS 16ULL // 10000000ULL at least TRANS_RATE_MCS /100
# define REQUIRED_RESET_SERVER_ACKS 2
# define RESET_SEQUENCE_COUNT 13
# define RESET_SEQUENCE_ATTEMPTS 4
# define BUFFER_SIZE_INCREMENT 32
# define CLNT_INACTIVITY_TMS 1000 //100000000 for debug, 1000 for prod

typedef struct s_buffer
{
	unsigned char	currnt_byte;
	int				currnt_bit;
	char			*buffr;
	size_t			size;
	size_t			len;
	int				expctd_bytes;
	int				reset_req_sequence;
	int				cmpltd_client_pid;
	int				inactvty_cnt_msc;
	int				purge_dirty;
}					t_buffer;

#endif
