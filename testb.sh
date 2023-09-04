#gcc test.c -I../ -L../ -l:ft_printf.a -I../../libft -L../../libft -lft -g -o ../../debugger/a.out
gcc bonus/server.c -I../ft_printf -L../ft_printf -l:ft_printf.a -I../libft -L../libft -lft -g -o ../debugger/server.out
gcc bonus/client.c -I../ft_printf -L../ft_printf -l:ft_printf.a -I../libft -L../libft -lft -g -o ../debugger/client.out
