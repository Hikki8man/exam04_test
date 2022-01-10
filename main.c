#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

typedef struct s_cmd
{
	char **param;
	int in;
	int out;
	struct s_cmd *next;
}t_cmd;

int		ft_strlen(char *s)
{
	int i = 0;

	if (!s)
		return 0;
	while (s[i])
		i++;
	return i;
}

void	ft_putstr_fd(char *s, int fd)
{
	write(fd, s, ft_strlen(s));
}

void	add_cmd(t_cmd **cmd, char **av, int i, int i_start)
{
	t_cmd *new;
	t_cmd *ptr;

	new = malloc(sizeof(t_cmd));
	av[i] = 0;
	new->param = av + i_start; 
	new->in = 0;
	new->out = 1;
	new->next = NULL;
	ptr = *cmd;
	if (!*cmd)
		*cmd = new;
	else
	{
		while (ptr->next)
			ptr = ptr->next;
		ptr->next = new;
	}
}


char	*ft_strjoin(char *s1, char *s2)
{
	char *ret;
	int i = 0;

	ret = malloc(sizeof(char) * (ft_strlen(s1) + ft_strlen(s2) + 1));
	if (!ret)
		return NULL;
	if (s1)
	{
		while (s1[i])
		{
			ret[i] = s1[i];
			i++;
		}
	}
	int j = 0;
	if (s2)
	{
		while (s2[j])
		{
			ret[i] = s2[j];
			j++;
			i++;
		}
	}
	ret[i] = 0;
	return ret;
}

void exec_cmd(t_cmd *cmd, int pipein, char **env)
{
	if (cmd->next)
		close(pipein);
	if (cmd->in != 0)
		dup2(cmd->in, 0);
	if (cmd->out != 1)
		dup2(cmd->out, 1);
	close(cmd->in);
	close(cmd->out);
	if (execve(*cmd->param, cmd->param, env) == -1)
	{
		ft_putstr_fd("error: cannot execute ", 2);
		ft_putstr_fd(*cmd->param, 2);
		ft_putstr_fd("\n", 2);
		exit(1);
	}
}

void	launch_process(t_cmd *cmd, char *sep, char **env)
{
	int *pipe_fd;
	int pid;
	int i = 0;

	while (cmd)
	{
		if (i != 0)
		{
			cmd->in = pipe_fd[0];
		}
		if (pipe(pipe_fd) == -1)
			return ;
		if (cmd->next)
			cmd->out = pipe_fd[1];
		else
		{
			close(pipe_fd[0]);
			close(pipe_fd[1]);
		}
		pid = fork();
		if (pid == 0)
			exit(0);
			// exec_cmd(cmd, pipe_fd[0], env);
		else if (pid == -1)
			return ;
		if (cmd->next)
			close(pipe_fd[1]);
		if (cmd->in != 0)
			close(cmd->in);
		if (i != 0)
		if (sep)
		{
			if (*sep == ';')
			{
				while (wait(NULL) != -1)
					;
			}
			sep++;
		}
		cmd = cmd->next;
		i++;
	}
	while (wait(NULL) != -1)
		;
}

int main(int ac, char **av, char **env)
{
	t_cmd	*cmd;
	int		i = 0;
	int		i_start = 0;
	char	*sep;
	char	*tmp;

	sep = NULL;
	cmd = NULL;
	av++;

	if (ac > 1)
	{
		while (av[i])
		{
			if (av[i][0] == '|' || av[i][0] == ';')
			{
				tmp = sep;
				sep = ft_strjoin(sep, av[i]);
				if (!sep)
					return (1);
				if (tmp)
					free(tmp);
				add_cmd(&cmd, av, i, i_start);
				i_start = i + 1;
			}
			i++;
		}
		add_cmd(&cmd, av, i, i_start);
		t_cmd *save = cmd; 
		launch_process(cmd, sep, env);


		int nb = 1;
		printf("SEP %s\n", sep);
		while (save)
		{
			i = 0;
			printf("CMD %d\n", nb);
			while (save->param[i])
			{
				printf("%s\n", save->param[i]);
				i++;
			}
			printf("----------------------\n");
			nb++;
			save = save->next;
		}
	}
}