/*
 * IP2Location C library is distributed under MIT license
 * Copyright (c) 2013-2020 IP2Location.com. support at ip2location dot com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the MIT license
 */

#ifndef HAVE_IP2LOCATION_H
#define HAVE_IP2LOCATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#if !defined(__APPLE__)
#include <stdlib.h>
#endif

#ifdef _WIN32
#define int16_t short
#define int32_t int
#define int64_t long long int
#else
#include <stdint.h>
#endif

#ifndef uint8_t
#define uint8_t unsigned char
#endif

#ifndef uint16_t
#define uint16_t short
#endif

#ifndef int32_t
#define int32_t int
#endif

#ifndef int64_t
#define int64_t long long int
#endif

#ifndef uint32_t
#ifndef _WIN32
#define uint32_t int
#else
#define uint32_t unsigned int
#endif
#endif

#define API_VERSION			8.3.1
#define API_VERSION_MAJOR	8
#define API_VERSION_MINOR	3
#define API_VERSION_RELEASE	1
#define API_VERSION_NUMERIC (((API_VERSION_MAJOR * 100) + API_VERSION_MINOR) * 100 + API_VERSION_RELEASE)

#define MAX_IPV4_RANGE	4294967295U
#define MAX_IPV6_RANGE	"340282366920938463463374607431768211455"
#define IPV4	0
#define IPV6	1

#define COUNTRYSHORT		0x00001
#define COUNTRYLONG			0x00002
#define REGION				0x00004
#define CITY				0x00008
#define ISP					0x00010
#define LATITUDE			0x00020
#define LONGITUDE			0x00040
#define DOMAINNAME			0x00080
#define ZIPCODE				0x00100
#define TIMEZONE			0x00200
#define NETSPEED			0x00400
#define IDDCODE				0x00800
#define AREACODE			0x01000
#define WEATHERSTATIONCODE	0x02000
#define WEATHERSTATIONNAME	0x04000
#define MCC					0x08000
#define MNC					0x10000
#define MOBILEBRAND			0x20000
#define ELEVATION			0x40000
#define USAGETYPE			0x80000

#define DEFAULT				0x0001
#define NO_EMPTY_STRING		0x0002
#define NO_LEADING			0x0004
#define NO_TRAILING			0x0008

#define ALL COUNTRYSHORT | COUNTRYLONG | REGION | CITY | ISP | LATITUDE | LONGITUDE | DOMAINNAME | ZIPCODE | TIMEZONE | NETSPEED | IDDCODE | AREACODE | WEATHERSTATIONCODE | WEATHERSTATIONNAME | MCC | MNC | MOBILEBRAND | ELEVATION | USAGETYPE
#define INVALID_IP_ADDRESS "INVALID IP ADDRESS"
#define IPV6_ADDRESS_MISSING_IN_IPV4_BIN "IPV6 ADDRESS MISSING IN IPV4 BIN"
#define NOT_SUPPORTED "This parameter is unavailable for selected data file. Please upgrade the data file."
#define IP2LOCATION_SHM "IP2location_Shm"
#define MAP_ADDR 4194500608

enum IP2Location_lookup_mode {
	IP2LOCATION_FILE_IO,
	IP2LOCATION_CACHE_MEMORY,
	IP2LOCATION_SHARED_MEMORY
};

typedef struct {
	FILE *file;
	uint8_t database_type;
	uint8_t database_column;
	uint8_t database_day;
	uint8_t database_month;
	uint8_t database_year;
	uint32_t database_count;
	uint32_t database_address;
	uint32_t ip_version;
	uint32_t ipv4_database_count;
	uint32_t ipv4_database_address;
	uint32_t ipv6_database_count;
	uint32_t ipv6_database_address;
	uint32_t ipv4_index_base_address;
	uint32_t ipv6_index_base_address;
} IP2Location;

typedef struct {
	char *country_short;
	char *country_long;
	char *region;
	char *city;
	char *isp;
	char *domain;
	char *zipcode;
	char *timezone;
	char *netspeed;
	char *iddcode;
	char *areacode;
	char *weatherstationcode;
	char *weatherstationname;
	char *mcc;
	char *mnc;
	char *mobilebrand;
	char *usagetype;

	float latitude;
	float longitude;
	float elevation;

	/* Variables changed since 8.1.0 */
	char *zip_code;
	char *time_zone;
	char *net_speed;
	char *idd_code;
	char *area_code;
	char *weather_station_code;
	char *weather_station_name;
	char *mobile_brand;
	char *usage_type;
} IP2LocationRecord;

typedef struct ip_container {
	uint32_t version;
	uint32_t ipv4;
	struct in6_addr ipv6;
} ip_container;

/* Public functions */
IP2Location *IP2Location_open(char *bin);

#if !defined(MSDOS)
  int32_t IP2Location_open_mem(IP2Location *handler, enum IP2Location_lookup_mode);
  void    IP2Location_delete_shm();
  void    IP2Location_DB_del_shm();
  int32_t IP2Location_set_lookup_mode(IP2Location *handler, enum IP2Location_lookup_mode);
  int32_t IP2Location_set_memory_cache(FILE* file);
  int32_t IP2Location_set_shared_memory(FILE* file);
#endif

uint32_t IP2Location_close(IP2Location *handler);
IP2LocationRecord *IP2Location_get_country_short(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_country_long(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_region(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_city (IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_isp(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_latitude(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_longitude(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_domain(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_zipcode(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_timezone(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_netspeed(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_iddcode(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_areacode(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_weatherstationcode(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_weatherstationname(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_mcc(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_mnc(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_mobilebrand(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_elevation(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_usagetype(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_all(IP2Location *handler, char *ip);
IP2LocationRecord *IP2Location_get_ipv4_record(IP2Location *handler, uint32_t mode, ip_container parsed_ip);
IP2LocationRecord *IP2Location_get_ipv6_record(IP2Location *handler, uint32_t mode, ip_container parsed_ip);

void IP2Location_free_record(IP2LocationRecord *record);
unsigned long int IP2Location_api_version_num(void);
char *IP2Location_api_version_string(void);
char *IP2Location_lib_version_string(void);
char *IP2Location_bin_version(IP2Location *handler);

struct in6_addr IP2Location_readIPv6Address(FILE *handle, uint32_t position);
uint32_t IP2Location_read32(FILE *handle, uint32_t position);
uint8_t IP2Location_read8(FILE *handle, uint32_t position);
char *IP2Location_readStr(FILE *handle, uint32_t position);
float IP2Location_readFloat(FILE *handle, uint32_t position);
int32_t IP2Location_DB_set_file_io();
int32_t IP2Location_DB_set_memory_cache(FILE *file);
int32_t IP2Location_DB_set_shared_memory(FILE *file);
int32_t IP2Location_DB_close(FILE *file);

unsigned long int IP2Location_api_version_number(void);
void IP2Location_clear_memory();
int32_t IP2Location_close_memory(FILE* file);
void IP2Location_delete_shared_memory();
float IP2Location_read_float(FILE* handle, uint32_t position);
struct in6_addr IP2Location_read_ipv6_address(FILE* handle, uint32_t position);
char *IP2Location_read_string(FILE* handle, uint32_t position);

#ifdef __cplusplus
}
#endif
#endif
