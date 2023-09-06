/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minitalk.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amarabin <amarabin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/02/21 20:15:54 by prossi            #+#    #+#             */
/*   Updated: 2023/09/06 19:24:15 by amarabin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINITALK_H
# define MINITALK_H

# include "../../ft_printf/ft_printf.h"
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

typedef struct s_buffer
{
	unsigned char	current_byte;
	int				current_bit;
}					t_buffer;

#endif
