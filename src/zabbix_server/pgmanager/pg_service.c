/*
** Zabbix
** Copyright (C) 2001-2023 Zabbix SIA
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#include "pg_service.h"
#include "zbxnix.h"

static void	*pg_service_entry(void *data)
{
	zbx_pg_service_t	*pgs = (zbx_pg_service_t *)data;
	zbx_timespec_t		timeout = {1, 0};
	zbx_ipc_client_t	*client;
	zbx_ipc_message_t	*message;

	zabbix_log(LOG_LEVEL_DEBUG, "In %s()", __func__);

	while (ZBX_IS_RUNNING())
	{
		(void)zbx_ipc_service_recv(&pgs->service, &timeout, &client, &message);

		if (NULL != message)
		{
			switch (message->code)
			{
				case ZBX_IPC_PGM_STOP:
					goto out;
			}

			zbx_ipc_message_free(message);
		}

		if (NULL != client)
			zbx_ipc_client_release(client);
	}
out:
	zabbix_log(LOG_LEVEL_DEBUG, "End of %s()", __func__);

	return NULL;
}

int	pg_service_init(zbx_pg_service_t *pgs, char **error)
{
	int	ret = FAIL;

	zabbix_log(LOG_LEVEL_DEBUG, "In %s()", __func__);

	if (FAIL == zbx_ipc_service_start(&pgs->service, ZBX_IPC_SERVICE_PG_MANAGER, error))
		goto out;

	pthread_attr_t	attr;
	int		err;

	zbx_pthread_init_attr(&attr);
	if (0 != (err = pthread_create(&pgs->thread, &attr, pg_service_entry, (void *)pgs)))
	{
		*error = zbx_dsprintf(NULL, "cannot create thread: %s", zbx_strerror(err));
		goto out;
	}

	ret = SUCCEED;
out:
	zabbix_log(LOG_LEVEL_DEBUG, "End of %s():%s", __func__, zbx_result_string(ret));

	return SUCCEED;
}

void	pg_service_destroy(zbx_pg_service_t *pgs)
{
	zbx_ipc_socket_t	sock;
	char			*error = NULL;

	if (FAIL == zbx_ipc_socket_open(&sock, ZBX_IPC_SERVICE_PG_MANAGER, 0, &error))
	{
		zabbix_log(LOG_LEVEL_ERR, "cannot connect to to proxy group manager service: %s", error);
		zbx_free(error);
		return;
	}

	zbx_ipc_socket_write(&sock, ZBX_IPC_PGM_STOP, NULL, 0);
	zbx_ipc_socket_close(&sock);

	void	*retval;

	pthread_join(pgs->thread, &retval);
}
