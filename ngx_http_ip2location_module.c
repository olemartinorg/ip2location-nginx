/*
 * COPYRIGHT (C) IP2LOCATION. ALL RIGHTS RESERVED.
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <stdio.h>

#include "../ip2location-c-7.0.0/libIP2Location/IP2Location.h"

/* Directive handlers */
static char *ngx_http_ip2location_database(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

/* Variable handlers */
static ngx_int_t ngx_http_ip2location_json(ngx_http_request_t *r,  ngx_http_variable_value_t *v, uintptr_t data);

/* Directives */
static ngx_command_t  ngx_http_ip2location_commands[] = {
	{ ngx_string("ip2location_database"),
	  NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
	  ngx_http_ip2location_database,
	  NGX_HTTP_MAIN_CONF_OFFSET,
	  0,
	  NULL },

	ngx_null_command
};

  
static ngx_http_module_t  ngx_http_ip2location_module_ctx = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


ngx_module_t  ngx_http_ip2location_module = {
	NGX_MODULE_V1,
	&ngx_http_ip2location_module_ctx,
	ngx_http_ip2location_commands,
	NGX_HTTP_MODULE,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_ip2location_json(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data){
	ngx_http_variable_value_t *vv = v;

	const char *format = "{\"country_long\":\"%s\",\"country_short\":\"%s\",\"city\":\"%s\",\"region\":\"%s\",\"lat\":%.5f,\"long\":%.5f}";

	IP2LocationRecord *record = IP2Location_get_all((IP2Location *)data, (char *)r->connection->addr_text.data);

	int len = snprintf(NULL, 0, format, record->country_long, record->country_short, record->city, record->region, record->latitude, record->longitude);

	char *str;
	if (!(str = malloc((len + 1) * sizeof(char))))
		return NGX_ERROR;

	snprintf(str, len + 1, format, record->country_long, record->country_short, record->city, record->region, record->latitude, record->longitude);

	vv->data = (u_char *)str;

	if (vv->data == NULL) {
		vv->valid = 0;
		vv->not_found = 1;
	} else {
		vv->len = ngx_strlen( vv->data );
		vv->valid = 1;
		vv->no_cacheable = 0;
		vv->not_found = 0;
	}

	return NGX_OK;
}

static char *
ngx_http_ip2location_database(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_str_t *value, bin_file;
	
	ngx_http_variable_t *json_var;
	ngx_str_t ngx_json_var_name = ngx_string("ip2location_json");

	value = cf->args->elts;

	bin_file = value[1];

	IP2Location *IP2LocationObj = IP2Location_open((char *)bin_file.data);

	IP2Location_open_mem(IP2LocationObj, IP2LOCATION_CACHE_MEMORY);

	if(IP2LocationObj == NULL)
		return NGX_CONF_ERROR;

	json_var = ngx_http_add_variable(cf, &ngx_json_var_name, NGX_HTTP_VAR_CHANGEABLE );
	if (json_var == NULL) {
		return NGX_CONF_ERROR;
	}

	json_var->get_handler = ngx_http_ip2location_json;
	json_var->data = (uintptr_t)IP2LocationObj;

	return NGX_CONF_OK;
}
