#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

typedef enum e_type {
	END,
	PIPE,
	BREAK
}type;

typedef struct s_cmd
{
	char **param;
	int type;
	int in;
	int out;
	struct s_cmd *prev;
	struct s_cmd *next;
}t_cmd;

void exit_fatal(void)
{
	exit (EXIT_FAILURE);
}

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

void	add_cmd(t_cmd **cmd, char **av, int i, int i_start, char t)
{
	t_cmd *new;
	t_cmd *ptr;

	new = malloc(sizeof(t_cmd));
	av[i] = 0;
	new->param = av + i_start; 
	new->in = 0;
	new->out = 1;
	new->prev = NULL;
	if (t == ';')
		new->type = BREAK;
	else if (t == '|')
		new->type = PIPE;
	else
		new->type = END;
	new->next = NULL;
	ptr = *cmd;
	if (!*cmd)
		*cmd = new;
	else
	{
		while (ptr->next)
			ptr = ptr->next;
		ptr->next = new;
		new->prev = ptr;
	}
}

void cd(t_cmd *cmd)
{
	if (!cmd->param[1])
	{
		ft_putstr_fd("error: cd: bad arguments\n", 2);
		exit (1);
	}
	if (chdir(cmd->param[1]) == -1)
	{
		ft_putstr_fd("error: cd: cannot change directory to ", 2);
		ft_putstr_fd(cmd->param[1], 2);
		ft_putstr_fd("\n", 2);
		exit (1);
	}
}

void	launch_process(t_cmd *cmd, char **env)
{
	int pipe_fd[2];
	int pid;
	while (cmd)
	{
		if (cmd->prev && cmd->prev->type == PIPE)
			cmd->in = pipe_fd[0];
		if (cmd->type == PIPE)
		{
			if (pipe(pipe_fd) == -1)
					exit_fatal();
			cmd->out = pipe_fd[1];
		}
		pid = fork();
		if (pid == -1)
			exit_fatal();
		else if (pid == 0)
		{
			if (cmd->type == PIPE)
				close(pipe_fd[0]);
			if (cmd->in != 0)
			{
				if (dup2(cmd->in, 0) == -1)
					exit_fatal();
				close(cmd->in);
			}
			if (cmd->out != 1)
			{
				if (dup2(cmd->out, 1) == -1)
					exit_fatal();
				close(cmd->out);
			}
			if (strcmp(cmd->param[0], "cd") == 0)
				cd(cmd);
			else if (execve(cmd->param[0], cmd->param, env) == -1)
			{
				//error blabla
				exit(1);
			}
		}
		if (cmd->type == PIPE)
			close(pipe_fd[1]);
		if (cmd->in != 0)
			close(cmd->in);
		if (cmd->type == BREAK)
			while (waitpid(0, NULL, 0) != -1)
				;
		cmd = cmd->next;
	}
	while (wait(NULL) != -1)
		;
}

int main(int ac, char **av, char **env)
{
	t_cmd	*cmd;
	int		i = 0;
	int		i_start = 0;

	cmd = NULL;
	av++;

	if (ac > 1)
	{
		while (av[i])
		{
			if ((av[i][0] == '|' && !av[i][1]) || (av[i][0] == ';' && !av[i][1]))
			{
				if (!av[i + 1] || av[i][0] == ';')
					exit(1);
				add_cmd(&cmd, av, i, i_start, av[i][0]);
				i_start = i + 1;	
			}
			i++;
		}
		add_cmd(&cmd, av, i, i_start, 0);
		launch_process(cmd, env);
	}
}