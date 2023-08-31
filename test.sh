#gcc test.c -I../ -L../ -l:ft_printf.a -I../../libft -L../../libft -lft -g -o ../../debugger/a.out
gcc srcs/server.c -I../ft_printf -L../ft_printf -l:ft_printf.a -I../libft -L../libft -lft -g -o ../debugger/server.out
gcc srcs/client.c -I../ft_printf -L../ft_printf -l:ft_printf.a -I../libft -L../libft -lft -g -o ../debugger/client.out
