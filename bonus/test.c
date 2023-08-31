static int	power_of_two(int pow)
{
	int	res;

	res = 1;
	while (pow > 0)
	{
		res = res * 2;
		pow--;
	}
	return (res);
}

static char	convert_bin_to_char(int *signal_collector)
{
	int	i;
	int	c;

	i = 8;
	c = 0;
	while (i >= 0)
	{
		if (signal_collector[i] == 1)
			c += power_of_two(i);
		i--;
	}
	return ((char)c);
}

static void	additional(int *j, char **str, int *realloc_counter)
{
	if ((*str)[*j] == 0)
	{
		ft_putendl(*str);
		free(*str);
		*str = NULL;
		*j = -1;
		*realloc_counter = STR_MAX_LEN;
	}
}

static void	decode(int sig)
{
	static int	signal_collector[8];
	static int	i = 0;
	static int	j = 0;
	static char	*str = NULL;
	static int	realloc_counter = STR_MAX_LEN;

	if (!str)
		str = malloc(sizeof(char) * realloc_counter);
	if (sig == SIGUSR1)
		signal_collector[i++] = 1;
	else if (sig == SIGUSR2)
		signal_collector[i++] = 0;
	if (i > 7)
	{
		if (j == (realloc_counter - 1))
		{
			str[j] = 0;
			str = ft_realloc(str, STR_MAX_LEN);
			realloc_counter += STR_MAX_LEN;
		}
		str[j] = convert_bin_to_char(signal_collector);
		additional(&j, &str, &realloc_counter);
		i = 0;
		j++;
	}
}

int	main(void)
{
	pid_t	pid;

	pid = getpid();
	put_menu_server(pid);
	while (1)
	{
		signal(SIGUSR1, decode);
		signal(SIGUSR2, decode);
		pause();
	}
	return (EXIT_SUCCESS);
}
