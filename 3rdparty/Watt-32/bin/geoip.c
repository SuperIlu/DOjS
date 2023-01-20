/* GeoIP.c
 *
 * Copyright (C) 2006 MaxMind LLC
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <io.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <malloc.h>
#include <crtdbg.h>
#endif

#if !defined(_MSC_VER) && !defined(__BORLANDC__)
#include <unistd.h>
#endif

#if defined(__CYGWIN__)
#include <sys/mman.h>
#endif

#if defined(__MINGW32__) || defined(__CYGWIN__) || defined(__DJGPP__)
#include <sys/time.h>
#endif

#if defined(USE_GEOIP)

#include <tcp.h>
#include "geoip.h"

#if !defined(_SSIZE_T_DEFINED) && (defined(_MSC_VER) || defined(__HIGHC__))
  #define _SSIZE_T_DEFINED
  #undef ssize_t
  #ifdef _WIN64
    typedef __int64 ssize_t;
  #else
    typedef int ssize_t;
  #endif
#endif

#if !defined(socklen_t)         /* <sys/wtypes.h> not included */
#define socklen_t int
#endif

#if defined(WIN32)
  #include <windows.h>
  #ifdef _MSC_VER
  #define snprintf _snprintf
  #endif
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic ignored  "-Wsign-compare"
#endif

#if !defined(_WIN32) && !defined(__MSDOS__) && !defined(MSDOS) || defined(__CYGWIN__)
#define USE_MMAP
#endif

static geoipv6_t IPV6_NULL;

#ifndef INADDR_NONE
#define INADDR_NONE  -1
#endif

#define COUNTRY_BEGIN           16776960
#define LARGE_COUNTRY_BEGIN     16515072
#define STATE_BEGIN_REV0        16700000
#define STATE_BEGIN_REV1        16000000
#define STRUCTURE_INFO_MAX_SIZE 20
#define DATABASE_INFO_MAX_SIZE  100
#define MAX_ORG_RECORD_LENGTH   300
#define US_OFFSET               1
#define CANADA_OFFSET           677
#define WORLD_OFFSET            1353
#define FIPS_RANGE              360

const char GeoIP_country_code[254][3] = { "--",
  "AP", "EU", "AD", "AE", "AF", "AG", "AI", "AL", "AM", "CW",
  "AO", "AQ", "AR", "AS", "AT", "AU", "AW", "AZ", "BA", "BB",
  "BD", "BE", "BF", "BG", "BH", "BI", "BJ", "BM", "BN", "BO",
  "BR", "BS", "BT", "BV", "BW", "BY", "BZ", "CA", "CC", "CD",
  "CF", "CG", "CH", "CI", "CK", "CL", "CM", "CN", "CO", "CR",
  "CU", "CV", "CX", "CY", "CZ", "DE", "DJ", "DK", "DM", "DO",
  "DZ", "EC", "EE", "EG", "EH", "ER", "ES", "ET", "FI", "FJ",
  "FK", "FM", "FO", "FR", "SX", "GA", "GB", "GD", "GE", "GF",
  "GH", "GI", "GL", "GM", "GN", "GP", "GQ", "GR", "GS", "GT",
  "GU", "GW", "GY", "HK", "HM", "HN", "HR", "HT", "HU", "ID",
  "IE", "IL", "IN", "IO", "IQ", "IR", "IS", "IT", "JM", "JO",
  "JP", "KE", "KG", "KH", "KI", "KM", "KN", "KP", "KR", "KW",
  "KY", "KZ", "LA", "LB", "LC", "LI", "LK", "LR", "LS", "LT",
  "LU", "LV", "LY", "MA", "MC", "MD", "MG", "MH", "MK", "ML",
  "MM", "MN", "MO", "MP", "MQ", "MR", "MS", "MT", "MU", "MV",
  "MW", "MX", "MY", "MZ", "NA", "NC", "NE", "NF", "NG", "NI",
  "NL", "NO", "NP", "NR", "NU", "NZ", "OM", "PA", "PE", "PF",
  "PG", "PH", "PK", "PL", "PM", "PN", "PR", "PS", "PT", "PW",
  "PY", "QA", "RE", "RO", "RU", "RW", "SA", "SB", "SC", "SD",
  "SE", "SG", "SH", "SI", "SJ", "SK", "SL", "SM", "SN", "SO",
  "SR", "ST", "SV", "SY", "SZ", "TC", "TD", "TF", "TG", "TH",
  "TJ", "TK", "TM", "TN", "TO", "TL", "TR", "TT", "TV", "TW",
  "TZ", "UA", "UG", "UM", "US", "UY", "UZ", "VA", "VC", "VE",
  "VG", "VI", "VN", "VU", "WF", "WS", "YE", "YT", "RS", "ZA",
  "ZM", "ME", "ZW", "A1", "A2", "O1", "AX", "GG", "IM", "JE",
  "BL", "MF", "BQ"
};

static const unsigned num_GeoIP_countries = (unsigned) (sizeof (GeoIP_country_code) / sizeof (GeoIP_country_code[0]));

#if defined(NOT_USED_IN_WATT32_TRACERT)

const char GeoIP_country_code3[254][4] = { "--",
  "AP",  "EU",  "AND", "ARE", "AFG", "ATG", "AIA", "ALB", "ARM", "CUW",
  "AGO", "ATA", "ARG", "ASM", "AUT", "AUS", "ABW", "AZE", "BIH", "BRB",
  "BGD", "BEL", "BFA", "BGR", "BHR", "BDI", "BEN", "BMU", "BRN", "BOL",
  "BRA", "BHS", "BTN", "BVT", "BWA", "BLR", "BLZ", "CAN", "CCK", "COD",
  "CAF", "COG", "CHE", "CIV", "COK", "CHL", "CMR", "CHN", "COL", "CRI",
  "CUB", "CPV", "CXR", "CYP", "CZE", "DEU", "DJI", "DNK", "DMA", "DOM",
  "DZA", "ECU", "EST", "EGY", "ESH", "ERI", "ESP", "ETH", "FIN", "FJI",
  "FLK", "FSM", "FRO", "FRA", "SXM", "GAB", "GBR", "GRD", "GEO", "GUF",
  "GHA", "GIB", "GRL", "GMB", "GIN", "GLP", "GNQ", "GRC", "SGS", "GTM",
  "GUM", "GNB", "GUY", "HKG", "HMD", "HND", "HRV", "HTI", "HUN", "IDN",
  "IRL", "ISR", "IND", "IOT", "IRQ", "IRN", "ISL", "ITA", "JAM", "JOR",
  "JPN", "KEN", "KGZ", "KHM", "KIR", "COM", "KNA", "PRK", "KOR", "KWT",
  "CYM", "KAZ", "LAO", "LBN", "LCA", "LIE", "LKA", "LBR", "LSO", "LTU",
  "LUX", "LVA", "LBY", "MAR", "MCO", "MDA", "MDG", "MHL", "MKD", "MLI",
  "MMR", "MNG", "MAC", "MNP", "MTQ", "MRT", "MSR", "MLT", "MUS", "MDV",
  "MWI", "MEX", "MYS", "MOZ", "NAM", "NCL", "NER", "NFK", "NGA", "NIC",
  "NLD", "NOR", "NPL", "NRU", "NIU", "NZL", "OMN", "PAN", "PER", "PYF",
  "PNG", "PHL", "PAK", "POL", "SPM", "PCN", "PRI", "PSE", "PRT", "PLW",
  "PRY", "QAT", "REU", "ROU", "RUS", "RWA", "SAU", "SLB", "SYC", "SDN",
  "SWE", "SGP", "SHN", "SVN", "SJM", "SVK", "SLE", "SMR", "SEN", "SOM",
  "SUR", "STP", "SLV", "SYR", "SWZ", "TCA", "TCD", "ATF", "TGO", "THA",
  "TJK", "TKL", "TKM", "TUN", "TON", "TLS", "TUR", "TTO", "TUV", "TWN",
  "TZA", "UKR", "UGA", "UMI", "USA", "URY", "UZB", "VAT", "VCT", "VEN",
  "VGB", "VIR", "VNM", "VUT", "WLF", "WSM", "YEM", "MYT", "SRB", "ZAF",
  "ZMB", "MNE", "ZWE", "A1", "A2", "O1", "ALA", "GGY", "IMN", "JEY",
  "BLM", "MAF", "BES"
};
#endif

const char *GeoIP_utf8_country_name[254] = {
    "N/A", "Asia/Pacific Region", "Europe", "Andorra", "United Arab Emirates", "Afghanistan", "Antigua and Barbuda",
    "Anguilla", "Albania", "Armenia", "Cura" "\xc3\xa7 " "ao",
    "Angola", "Antarctica", "Argentina", "American Samoa", "Austria", "Australia", "Aruba", "Azerbaijan",
    "Bosnia and Herzegovina", "Barbados",
    "Bangladesh", "Belgium", "Burkina Faso", "Bulgaria", "Bahrain", "Burundi", "Benin", "Bermuda", "Brunei Darussalam",
    "Bolivia", "Brazil", "Bahamas", "Bhutan", "Bouvet Island", "Botswana", "Belarus", "Belize", "Canada", "Cocos (Keeling) Islands",
    "Congo, The Democratic Republic of the", "Central African Republic", "Congo", "Switzerland", "Cote D'Ivoire", "Cook Islands",
    "Chile", "Cameroon", "China", "Colombia", "Costa Rica", "Cuba", "Cape Verde", "Christmas Island", "Cyprus", "Czech Republic",
    "Germany", "Djibouti", "Denmark", "Dominica", "Dominican Republic", "Algeria", "Ecuador", "Estonia", "Egypt", "Western Sahara",
    "Eritrea", "Spain", "Ethiopia", "Finland", "Fiji", "Falkland Islands (Malvinas)", "Micronesia, Federated States of",
    "Faroe Islands", "France", "Sint Maarten (Dutch part)", "Gabon", "United Kingdom", "Grenada", "Georgia", "French Guiana",
    "Ghana", "Gibraltar", "Greenland", "Gambia", "Guinea", "Guadeloupe", "Equatorial Guinea", "Greece",
    "South Georgia and the South Sandwich Islands", "Guatemala", "Guam", "Guinea-Bissau", "Guyana", "Hong Kong",
    "Heard Island and McDonald Islands", "Honduras", "Croatia", "Haiti", "Hungary", "Indonesia", "Ireland", "Israel", "India",
    "British Indian Ocean Territory", "Iraq", "Iran, Islamic Republic of", "Iceland", "Italy", "Jamaica", "Jordan", "Japan",
    "Kenya", "Kyrgyzstan", "Cambodia", "Kiribati", "Comoros", "Saint Kitts and Nevis", "Korea, Democratic People's Republic of",
    "Korea, Republic of", "Kuwait", "Cayman Islands", "Kazakhstan", "Lao People's Democratic Republic", "Lebanon", "Saint Lucia",
    "Liechtenstein", "Sri Lanka", "Liberia", "Lesotho", "Lithuania", "Luxembourg", "Latvia", "Libyan Arab Jamahiriya", "Morocco",
    "Monaco", "Moldova, Republic of", "Madagascar", "Marshall Islands", "Macedonia", "Mali", "Myanmar", "Mongolia", "Macau",
    "Northern Mariana Islands", "Martinique", "Mauritania", "Montserrat", "Malta", "Mauritius", "Maldives", "Malawi", "Mexico",
    "Malaysia", "Mozambique", "Namibia", "New Caledonia", "Niger", "Norfolk Island", "Nigeria", "Nicaragua", "Netherlands", "Norway",
    "Nepal", "Nauru", "Niue", "New Zealand", "Oman", "Panama", "Peru", "French Polynesia", "Papua New Guinea", "Philippines",
    "Pakistan", "Poland", "Saint Pierre and Miquelon", "Pitcairn Islands", "Puerto Rico", "Palestinian Territory", "Portugal",
    "Palau", "Paraguay", "Qatar", "Reunion", "Romania", "Russian Federation", "Rwanda", "Saudi Arabia", "Solomon Islands",
    "Seychelles", "Sudan", "Sweden", "Singapore", "Saint Helena", "Slovenia", "Svalbard and Jan Mayen", "Slovakia", "Sierra Leone",
    "San Marino", "Senegal", "Somalia", "Suriname", "Sao Tome and Principe", "El Salvador", "Syrian Arab Republic", "Swaziland",
    "Turks and Caicos Islands", "Chad", "French Southern Territories", "Togo", "Thailand", "Tajikistan", "Tokelau", "Turkmenistan",
    "Tunisia", "Tonga", "Timor-Leste", "Turkey", "Trinidad and Tobago", "Tuvalu", "Taiwan", "Tanzania, United Republic of",
    "Ukraine", "Uganda", "United States Minor Outlying Islands", "United States", "Uruguay", "Uzbekistan", "Holy See (Vatican City State)",
    "Saint Vincent and the Grenadines", "Venezuela", "Virgin Islands, British", "Virgin Islands, U.S.", "Vietnam", "Vanuatu",
    "Wallis and Futuna", "Samoa", "Yemen", "Mayotte", "Serbia", "South Africa", "Zambia", "Montenegro", "Zimbabwe", "Anonymous Proxy",
    "Satellite Provider", "Other", "Aland Islands", "Guernsey", "Isle of Man", "Jersey", "Saint Barthelemy", "Saint Martin",
    "Bonaire, Saint Eustatius and Saba"
};

const char *GeoIP_country_name[254] = {
  "N/A", "Asia/Pacific Region", "Europe", "Andorra", "United Arab Emirates", "Afghanistan", "Antigua and Barbuda",
  "Anguilla", "Albania", "Armenia", "Cura" "\xe7" "ao", "Angola", "Antarctica", "Argentina", "American Samoa",
  "Austria", "Australia", "Aruba", "Azerbaijan", "Bosnia and Herzegovina", "Barbados", "Bangladesh", "Belgium",
  "Burkina Faso", "Bulgaria", "Bahrain", "Burundi", "Benin", "Bermuda", "Brunei Darussalam", "Bolivia",
  "Brazil", "Bahamas", "Bhutan", "Bouvet Island", "Botswana", "Belarus", "Belize", "Canada", "Cocos (Keeling) Islands",
   "Congo, The Democratic Republic of the", "Central African Republic", "Congo", "Switzerland", "Cote D'Ivoire",
   "Cook Islands", "Chile", "Cameroon", "China", "Colombia", "Costa Rica", "Cuba", "Cape Verde", "Christmas Island",
   "Cyprus", "Czech Republic", "Germany", "Djibouti", "Denmark", "Dominica", "Dominican Republic", "Algeria", "Ecuador",
   "Estonia", "Egypt", "Western Sahara", "Eritrea", "Spain", "Ethiopia", "Finland", "Fiji", "Falkland Islands (Malvinas)",
   "Micronesia, Federated States of", "Faroe Islands", "France", "Sint Maarten (Dutch part)", "Gabon", "United Kingdom",
   "Grenada", "Georgia", "French Guiana", "Ghana", "Gibraltar", "Greenland", "Gambia", "Guinea", "Guadeloupe", "Equatorial Guinea",
   "Greece", "South Georgia and the South Sandwich Islands", "Guatemala", "Guam", "Guinea-Bissau", "Guyana", "Hong Kong",
   "Heard Island and McDonald Islands", "Honduras", "Croatia", "Haiti", "Hungary", "Indonesia", "Ireland", "Israel", "India",
   "British Indian Ocean Territory", "Iraq", "Iran, Islamic Republic of", "Iceland", "Italy", "Jamaica", "Jordan", "Japan",
   "Kenya", "Kyrgyzstan", "Cambodia", "Kiribati", "Comoros", "Saint Kitts and Nevis", "Korea, Democratic People's Republic of",
   "Korea, Republic of", "Kuwait", "Cayman Islands", "Kazakhstan", "Lao People's Democratic Republic", "Lebanon", "Saint Lucia",
   "Liechtenstein", "Sri Lanka", "Liberia", "Lesotho", "Lithuania", "Luxembourg", "Latvia", "Libyan Arab Jamahiriya", "Morocco",
   "Monaco", "Moldova, Republic of", "Madagascar", "Marshall Islands", "Macedonia", "Mali", "Myanmar", "Mongolia", "Macau",
   "Northern Mariana Islands", "Martinique", "Mauritania", "Montserrat", "Malta", "Mauritius", "Maldives", "Malawi", "Mexico",
   "Malaysia", "Mozambique", "Namibia", "New Caledonia", "Niger", "Norfolk Island", "Nigeria", "Nicaragua", "Netherlands", "Norway",
   "Nepal", "Nauru", "Niue", "New Zealand", "Oman", "Panama", "Peru", "French Polynesia", "Papua New Guinea", "Philippines",
   "Pakistan", "Poland", "Saint Pierre and Miquelon", "Pitcairn Islands", "Puerto Rico", "Palestinian Territory", "Portugal",
   "Palau", "Paraguay", "Qatar", "Reunion", "Romania", "Russian Federation", "Rwanda", "Saudi Arabia", "Solomon Islands",
    "Seychelles", "Sudan", "Sweden", "Singapore", "Saint Helena", "Slovenia", "Svalbard and Jan Mayen", "Slovakia", "Sierra Leone",
    "San Marino", "Senegal", "Somalia", "Suriname", "Sao Tome and Principe", "El Salvador", "Syrian Arab Republic", "Swaziland",
    "Turks and Caicos Islands", "Chad", "French Southern Territories", "Togo", "Thailand", "Tajikistan", "Tokelau", "Turkmenistan",
    "Tunisia", "Tonga", "Timor-Leste", "Turkey", "Trinidad and Tobago", "Tuvalu", "Taiwan", "Tanzania, United Republic of",
    "Ukraine", "Uganda", "United States Minor Outlying Islands", "United States", "Uruguay", "Uzbekistan", "Holy See (Vatican City State)",
    "Saint Vincent and the Grenadines", "Venezuela", "Virgin Islands, British", "Virgin Islands, U.S.", "Vietnam", "Vanuatu",
    "Wallis and Futuna", "Samoa", "Yemen", "Mayotte", "Serbia", "South Africa", "Zambia", "Montenegro", "Zimbabwe", "Anonymous Proxy",
    "Satellite Provider", "Other", "Aland Islands", "Guernsey", "Isle of Man", "Jersey", "Saint Barthelemy", "Saint Martin",
    "Bonaire, Saint Eustatius and Saba"
  };

/* Possible continent codes are AF, AS, EU, NA, OC, SA for Africa, Asia, Europe, North America, Oceania
 * and South America.
 */
const char GeoIP_country_continent[254][3] = {
  "--", "AS", "EU", "EU", "AS", "AS", "NA", "NA", "EU", "AS", "NA",
  "AF", "AN", "SA", "OC", "EU", "OC", "NA", "AS", "EU", "NA",
  "AS", "EU", "AF", "EU", "AS", "AF", "AF", "NA", "AS", "SA",
  "SA", "NA", "AS", "AN", "AF", "EU", "NA", "NA", "AS", "AF",
  "AF", "AF", "EU", "AF", "OC", "SA", "AF", "AS", "SA", "NA",
  "NA", "AF", "AS", "AS", "EU", "EU", "AF", "EU", "NA", "NA",
  "AF", "SA", "EU", "AF", "AF", "AF", "EU", "AF", "EU", "OC",
  "SA", "OC", "EU", "EU", "NA", "AF", "EU", "NA", "AS", "SA",
  "AF", "EU", "NA", "AF", "AF", "NA", "AF", "EU", "AN", "NA",
  "OC", "AF", "SA", "AS", "AN", "NA", "EU", "NA", "EU", "AS",
  "EU", "AS", "AS", "AS", "AS", "AS", "EU", "EU", "NA", "AS",
  "AS", "AF", "AS", "AS", "OC", "AF", "NA", "AS", "AS", "AS",
  "NA", "AS", "AS", "AS", "NA", "EU", "AS", "AF", "AF", "EU",
  "EU", "EU", "AF", "AF", "EU", "EU", "AF", "OC", "EU", "AF",
  "AS", "AS", "AS", "OC", "NA", "AF", "NA", "EU", "AF", "AS",
  "AF", "NA", "AS", "AF", "AF", "OC", "AF", "OC", "AF", "NA",
  "EU", "EU", "AS", "OC", "OC", "OC", "AS", "NA", "SA", "OC",
  "OC", "AS", "AS", "EU", "NA", "OC", "NA", "AS", "EU", "OC",
  "SA", "AS", "AF", "EU", "EU", "AF", "AS", "OC", "AF", "AF",
  "EU", "AS", "AF", "EU", "EU", "EU", "AF", "EU", "AF", "AF",
  "SA", "AF", "NA", "AS", "AF", "NA", "AF", "AN", "AF", "AS",
  "AS", "OC", "AS", "AF", "OC", "AS", "EU", "NA", "OC", "AS",
  "AF", "EU", "AF", "OC", "NA", "SA", "AS", "EU", "NA", "SA",
  "NA", "NA", "AS", "OC", "OC", "OC", "AS", "AF", "EU", "AF",
  "AF", "EU", "AF", "--", "--", "--", "EU", "EU", "EU", "EU",
  "NA", "NA", "NA"
};

geoipv6_t _GeoIP_lookupaddress_v6 (const char *host);

#if defined(_WIN32) || defined(__MSDOS__) || defined(MSDOS)
ssize_t pread (int fd, void *buf, size_t count, off_t offset)
{
  ssize_t retval;
  off_t   saved_pos = lseek (fd, 0, SEEK_CUR);

  lseek (fd, offset, SEEK_SET);
  retval = read (fd, buf, count);
  lseek (fd, saved_pos, SEEK_SET);
  return (retval);
}
#endif

#if defined(NOT_USED_IN_WATT32_TRACERT)

/*
 * http://www.mail-archive.com/users@ipv6.org/msg02107.html
 */
static const char *_GeoIP_inet_ntop (int af, const void *src, char *dst, socklen_t cnt)
{
  if (af == AF_INET)
  {
    struct sockaddr_in in;

    memset (&in, 0, sizeof(in));
    in.sin_family = AF_INET;
    memcpy (&in.sin_addr, src, sizeof (struct in_addr));
    getnameinfo ((struct sockaddr*)&in, sizeof(struct sockaddr_in), dst, cnt, NULL, 0, NI_NUMERICHOST);
    return (dst);
  }
  else if (af == AF_INET6)
  {
    struct sockaddr_in6 in;

    memset (&in, 0, sizeof (in));
    in.sin6_family = AF_INET6;
    memcpy (&in.sin6_addr, src, sizeof(struct in6_addr));
    getnameinfo ((struct sockaddr*)&in, sizeof(struct sockaddr_in6), dst, cnt, NULL, 0, NI_NUMERICHOST);
    return (dst);
  }
  return (NULL);
}

static int _GeoIP_inet_pton (int af, const char *src, void *dst)
{
  struct addrinfo hints, *res, *ressave;

  memset (&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = af;

  if (getaddrinfo(src, NULL, &hints, &res) != 0)
  {
    printf ("Couldn't resolve host %s\n", src);
    return (-1);
  }

  ressave = res;

  while (res)
  {
    memcpy (dst, res->ai_addr, res->ai_addrlen);
    res = res->ai_next;
  }

  freeaddrinfo (ressave);
  return (0);
}
#else

static int _GeoIP_inet_pton (int af, const char *src, void *dst)
{
  return inet_pton (af, src, dst);
}

static const char *_GeoIP_inet_ntop (int af, const void *src, char *dst, socklen_t cnt)
{
  return inet_ntop (af, src, dst, cnt);
}
#endif     /* NOT_USED_IN_WATT32_TRACERT */


int __GEOIP_V6_IS_NULL (geoipv6_t v6)
{
  int i;

  for (i = 0; i < 16; i++)
  {
    if (v6.s6_addr[i])
       return (0);
  }
  return (1);
}

void __GEOIP_PREPARE_TEREDO (geoipv6_t * v6)
{
  int i;

  if (v6->s6_addr[0] != 0x20)
     return;
  if (v6->s6_addr[1] != 0x01)
     return;
  if (v6->s6_addr[2] != 0x00)
     return;
  if (v6->s6_addr[3] != 0x00)
     return;

  for (i = 0; i < 12; i++)
      v6->s6_addr[i] = 0;
  for (; i < 16; i++)
      v6->s6_addr[i] ^= 0xff;
}

const char *GeoIPDBDescription[NUM_DB_TYPES] = {
  NULL,
  "GeoIP Country Edition",
  "GeoIP City Edition, Rev 1",
  "GeoIP Region Edition, Rev 1",
  "GeoIP ISP Edition",
  "GeoIP Organization Edition",
  "GeoIP City Edition, Rev 0",
  "GeoIP Region Edition, Rev 0",
  "GeoIP Proxy Edition",
  "GeoIP ASNum Edition",
  "GeoIP Netspeed Edition",
  "GeoIP Domain Name Edition",
  "GeoIP Country V6 Edition",
  "GeoIP LocationID ASCII Edition",
  "GeoIP Accuracy Radius Edition",
  "GeoIP City with Confidence Edition",
  "GeoIP City with Confidence and Accuracy Edition",
  "GeoIP Large Country Edition",
  "GeoIP Large Country V6 Edition",
  NULL,
  "GeoIP CCM Edition",
  "GeoIP ASNum V6 Edition",
  "GeoIP ISP V6 Edition",
  "GeoIP Organization V6 Edition",
  "GeoIP Domain Name V6 Edition",
  "GeoIP LocationID ASCII V6 Edition",
  "GeoIP Registrar Edition",
  "GeoIP Registrar V6 Edition",
  "GeoIP UserType Edition",
  "GeoIP UserType V6 Edition",
  "GeoIP City Edition V6, Rev 1",
  "GeoIP City Edition V6, Rev 0",
  "GeoIP Netspeed Edition, Rev 1",
  "GeoIP Netspeed Edition V6, Rev1"
};

#ifndef GEOIPDATADIR
#define GEOIPDATADIR getenv("GEOIPDATADIR")
#endif

const char *custom_directory = NULL;

void GeoIP_setup_custom_directory (const char *dir)
{
  custom_directory = dir;
}

#ifndef GEOIP_PATH_LEN
#define GEOIP_PATH_LEN 1024
#endif

char *_GeoIP_full_path_to (const char *file_name)
{
  int   len;
  char *path = calloc (GEOIP_PATH_LEN+1, 1);

  if (custom_directory == NULL)
  {
#if !defined(_WIN32)
    snprintf (path, GEOIP_PATH_LEN, "%s/%s", GEOIPDATADIR, file_name);
#else
    char buf[MAX_PATH], *p, *q = NULL;

    memset (buf, 0, sizeof(buf));
    len = GetModuleFileNameA (GetModuleHandle(NULL), buf, sizeof(buf) - 1);
    for (p = buf + len; p > buf; p--)
      if (*p == '\\')
      {
        if (!q)
             q = p;
        else *p = '/';
      }
    *q = 0;
    snprintf (path, GEOIP_PATH_LEN, "%s/%s", buf, file_name);
#endif
  }
  else
  {
    len = strlen (custom_directory);
    if (custom_directory[len-1] != '/')
    {
      snprintf (path, GEOIP_PATH_LEN, "%s/%s", custom_directory, file_name);
    }
    else
    {
      snprintf (path, GEOIP_PATH_LEN, "%s%s", custom_directory, file_name);
    }
  }
  return (path);
}

char **GeoIPDBFileName = NULL;

void _GeoIP_setup_dbfilename (void)
{
  if (!GeoIPDBFileName)
  {
    GeoIPDBFileName = calloc (sizeof(char*) * NUM_DB_TYPES, 1);

    GeoIPDBFileName [GEOIP_COUNTRY_EDITION]            = _GeoIP_full_path_to ("GeoIP.dat");
    GeoIPDBFileName [GEOIP_REGION_EDITION_REV0]        = _GeoIP_full_path_to ("GeoIPRegion.dat");
    GeoIPDBFileName [GEOIP_REGION_EDITION_REV1]        = _GeoIP_full_path_to ("GeoIPRegion.dat");
    GeoIPDBFileName [GEOIP_CITY_EDITION_REV0]          = _GeoIP_full_path_to ("GeoIPCity.dat");
    GeoIPDBFileName [GEOIP_CITY_EDITION_REV1]          = _GeoIP_full_path_to ("GeoIPCity.dat");
    GeoIPDBFileName [GEOIP_ISP_EDITION]                = _GeoIP_full_path_to ("GeoIPISP.dat");
    GeoIPDBFileName [GEOIP_ORG_EDITION]                = _GeoIP_full_path_to ("GeoIPOrg.dat");
    GeoIPDBFileName [GEOIP_PROXY_EDITION]              = _GeoIP_full_path_to ("GeoIPProxy.dat");
    GeoIPDBFileName [GEOIP_ASNUM_EDITION]              = _GeoIP_full_path_to ("GeoIPASNum.dat");
    GeoIPDBFileName [GEOIP_NETSPEED_EDITION]           = _GeoIP_full_path_to ("GeoIPNetSpeed.dat");
    GeoIPDBFileName [GEOIP_DOMAIN_EDITION]             = _GeoIP_full_path_to ("GeoIPDomain.dat");
    GeoIPDBFileName [GEOIP_COUNTRY_EDITION_V6]         = _GeoIP_full_path_to ("GeoIPv6.dat");
    GeoIPDBFileName [GEOIP_LOCATIONA_EDITION]          = _GeoIP_full_path_to ("GeoIPLocA.dat");
    GeoIPDBFileName [GEOIP_ACCURACYRADIUS_EDITION]     = _GeoIP_full_path_to ("GeoIPDistance.dat");
    GeoIPDBFileName [GEOIP_CITYCONFIDENCE_EDITION]     = _GeoIP_full_path_to ("GeoIPCityConfidence.dat");
    GeoIPDBFileName [GEOIP_CITYCONFIDENCEDIST_EDITION] = _GeoIP_full_path_to ("GeoIPCityConfidenceDist.dat");
    GeoIPDBFileName [GEOIP_LARGE_COUNTRY_EDITION]      = _GeoIP_full_path_to ("GeoIP.dat");
    GeoIPDBFileName [GEOIP_LARGE_COUNTRY_EDITION_V6]   = _GeoIP_full_path_to ("GeoIPv6.dat");
    GeoIPDBFileName [GEOIP_ASNUM_EDITION_V6]           = _GeoIP_full_path_to ("GeoIPASNumv6.dat");
    GeoIPDBFileName [GEOIP_ISP_EDITION_V6]             = _GeoIP_full_path_to ("GeoIPISPv6.dat");
    GeoIPDBFileName [GEOIP_ORG_EDITION_V6]             = _GeoIP_full_path_to ("GeoIPOrgv6.dat");
    GeoIPDBFileName [GEOIP_DOMAIN_EDITION_V6]          = _GeoIP_full_path_to ("GeoIPDomainv6.dat");
    GeoIPDBFileName [GEOIP_LOCATIONA_EDITION_V6]       = _GeoIP_full_path_to ("GeoIPLocAv6.dat");
    GeoIPDBFileName [GEOIP_REGISTRAR_EDITION]          = _GeoIP_full_path_to ("GeoIPRegistrar.dat");
    GeoIPDBFileName [GEOIP_REGISTRAR_EDITION_V6]       = _GeoIP_full_path_to ("GeoIPRegistrarv6.dat");
    GeoIPDBFileName [GEOIP_USERTYPE_EDITION]           = _GeoIP_full_path_to ("GeoIPUserType.dat");
    GeoIPDBFileName [GEOIP_USERTYPE_EDITION_V6]        = _GeoIP_full_path_to ("GeoIPUserTypev6.dat");
    GeoIPDBFileName [GEOIP_CITY_EDITION_REV0_V6]       = _GeoIP_full_path_to ("GeoIPCityv6.dat");
    GeoIPDBFileName [GEOIP_CITY_EDITION_REV1_V6]       = _GeoIP_full_path_to ("GeoIPCityv6.dat");
    GeoIPDBFileName [GEOIP_NETSPEED_EDITION_REV1]      = _GeoIP_full_path_to ("GeoIPNetspeedCell.dat");
    GeoIPDBFileName [GEOIP_NETSPEED_EDITION_REV1_V6]   = _GeoIP_full_path_to ("GeoIPNetseedCellv6.dat");
  }
}

static int _file_exists (const char *file_name)
{
  struct stat file_stat;

  return ((stat(file_name, &file_stat) == 0) ? 1 : 0);
}

char *_GeoIP_iso_8859_1__utf8 (const char *iso)
{
  signed char c;
  char  k, *p, *t = (char*)iso;
  int   len = 0;

  while ((c = *t++) != 0)
  {
    if (c < 0)
       len++;
  }
  len += t - iso;
  t = p = malloc (len);

  if (p)
  {
    while ((c = *iso++) != 0)
    {
      if (c < 0)
      {
        k = 0xc2;
        if (c >= -64)
           k++;
        *t++ = k;
        c &= ~0x40;
      }
      *t++ = c;
    }
    *t++ = 0x00;
  }
  return (p);
}

int GeoIP_is_private_ipnum_v4 (unsigned long ipnum)
{
  return ((ipnum >= 167772160U && ipnum <= 184549375U) ||
          (ipnum >= 2851995648U && ipnum <= 2852061183U) ||
          (ipnum >= 2886729728U && ipnum <= 2887778303U) ||
          (ipnum >= 3232235520U && ipnum <= 3232301055U) ||
          (ipnum >= 2130706432U && ipnum <= 2147483647U)) ? 1 : 0;
}

int GeoIP_is_private_v4 (const char *addr)
{
  unsigned long ipnum = GeoIP_addr_to_num (addr);

  return GeoIP_is_private_ipnum_v4 (ipnum);
}

int GeoIP_db_avail (int type)
{
  const char *filePath;

  if (type < 0 || type >= NUM_DB_TYPES)
     return (0);

  _GeoIP_setup_dbfilename();
  filePath = GeoIPDBFileName[type];
  if (!filePath)
     return (0);
  return _file_exists (filePath);
}

static void _setup_segments (GeoIP *gi)
{
  int           fno, i, j, segment_record_length;
  unsigned char delim[3];
  unsigned char buf[LARGE_SEGMENT_RECORD_LENGTH];

  fno = fileno (gi->GeoIPDatabase);
  gi->databaseSegments = NULL;

  /* default to GeoIP Country Edition
   */
  gi->databaseType = GEOIP_COUNTRY_EDITION;
  gi->record_length = STANDARD_RECORD_LENGTH;
  lseek (fno, -3l, SEEK_END);

  for (i = 0; i < STRUCTURE_INFO_MAX_SIZE; i++)
  {
    read (fno, delim, 3);
    if (delim[0] == 255 && delim[1] == 255 && delim[2] == 255)
    {
      read (fno, &gi->databaseType, 1);
      if (gi->databaseType >= 106)
      {
        /* backwards compatibility with databases from April 2003 and earlier */
        gi->databaseType -= 105;
      }
      if (gi->databaseType == GEOIP_REGION_EDITION_REV0)
      {
        /* Region Edition, pre June 2003 */
        gi->databaseSegments = malloc (sizeof (int));
        gi->databaseSegments[0] = STATE_BEGIN_REV0;
      }
      else if (gi->databaseType == GEOIP_REGION_EDITION_REV1)
      {
        /* Region Edition, post June 2003 */
        gi->databaseSegments = malloc (sizeof (int));
        gi->databaseSegments[0] = STATE_BEGIN_REV1;
      }
      else if (gi->databaseType == GEOIP_CITY_EDITION_REV0 ||
               gi->databaseType == GEOIP_CITY_EDITION_REV1 ||
               gi->databaseType == GEOIP_ORG_EDITION ||
               gi->databaseType == GEOIP_ORG_EDITION_V6 ||
               gi->databaseType == GEOIP_DOMAIN_EDITION ||
               gi->databaseType == GEOIP_DOMAIN_EDITION_V6 ||
               gi->databaseType == GEOIP_ISP_EDITION ||
               gi->databaseType == GEOIP_ISP_EDITION_V6 ||
               gi->databaseType == GEOIP_REGISTRAR_EDITION ||
               gi->databaseType == GEOIP_REGISTRAR_EDITION_V6 ||
               gi->databaseType == GEOIP_USERTYPE_EDITION ||
               gi->databaseType == GEOIP_USERTYPE_EDITION_V6 ||
               gi->databaseType == GEOIP_ASNUM_EDITION ||
               gi->databaseType == GEOIP_ASNUM_EDITION_V6 ||
               gi->databaseType == GEOIP_NETSPEED_EDITION_REV1 ||
               gi->databaseType == GEOIP_NETSPEED_EDITION_REV1_V6 ||
               gi->databaseType == GEOIP_LOCATIONA_EDITION ||
               gi->databaseType == GEOIP_ACCURACYRADIUS_EDITION ||
               gi->databaseType == GEOIP_CITYCONFIDENCE_EDITION ||
               gi->databaseType == GEOIP_CITYCONFIDENCEDIST_EDITION ||
               gi->databaseType == GEOIP_CITY_EDITION_REV0_V6 ||
               gi->databaseType == GEOIP_CITY_EDITION_REV1_V6)
      {
        /* City/Org Editions have two segments, read offset of second segment */
        gi->databaseSegments = malloc (sizeof(int));
        gi->databaseSegments[0] = 0;

        segment_record_length = (gi->databaseType == GEOIP_CITYCONFIDENCEDIST_EDITION) ?
                                 LARGE_SEGMENT_RECORD_LENGTH : SEGMENT_RECORD_LENGTH;

        read (fno, buf, segment_record_length);
        for (j = 0; j < segment_record_length; j++)
        {
          gi->databaseSegments[0] += (buf[j] << (j * 8));
        }

        /* the record_length must be correct from here on */
        if (gi->databaseType == GEOIP_ORG_EDITION ||
            gi->databaseType == GEOIP_ORG_EDITION_V6 ||
            gi->databaseType == GEOIP_DOMAIN_EDITION ||
            gi->databaseType == GEOIP_DOMAIN_EDITION_V6 ||
            gi->databaseType == GEOIP_ISP_EDITION ||
            gi->databaseType == GEOIP_ISP_EDITION_V6 ||
            gi->databaseType == GEOIP_CITYCONFIDENCEDIST_EDITION)
          gi->record_length = ORG_RECORD_LENGTH;

        if (gi->databaseType == GEOIP_CITYCONFIDENCE_EDITION ||
            gi->databaseType == GEOIP_CITYCONFIDENCEDIST_EDITION)
        {
          pread (fileno (gi->GeoIPDatabase), buf, gi->record_length, gi->databaseSegments[0] * 2 * gi->record_length);
          gi->dyn_seg_size = 0;
          for (j = 0; j < gi->record_length; j++)
          {
            gi->dyn_seg_size += (buf[j] << (j * 8));
          }
        }
      }
      break;
    }
    else
    {
      lseek (fno, -4l, SEEK_CUR);
    }
  }
  if (gi->databaseType == GEOIP_COUNTRY_EDITION ||
      gi->databaseType == GEOIP_PROXY_EDITION ||
      gi->databaseType == GEOIP_NETSPEED_EDITION ||
      gi->databaseType == GEOIP_COUNTRY_EDITION_V6)
  {
    gi->databaseSegments = malloc (sizeof(int));
    gi->databaseSegments[0] = COUNTRY_BEGIN;
  }
  else if (gi->databaseType == GEOIP_LARGE_COUNTRY_EDITION ||
           gi->databaseType == GEOIP_LARGE_COUNTRY_EDITION_V6)
  {
    gi->databaseSegments = malloc (sizeof(int));
    gi->databaseSegments[0] = LARGE_COUNTRY_BEGIN;
  }
}

static int _check_mtime (GeoIP *gi)
{
  struct stat    buf;
  struct timeval t;

  if (gi->flags & GEOIP_CHECK_CACHE)
  {
    /* stat() could have 1 second granularity, so don't
     * call it more than once a second
     */
    gettimeofday (&t, NULL);
    if (t.tv_sec == gi->last_mtime_check)
       return (0);

    gi->last_mtime_check = t.tv_sec;

    if (stat(gi->file_path, &buf) != -1)
    {
      /* make sure that the database file is at least 60
       * seconds untouched. Otherwise we might load the
       * database only partly and crash
       */
      if (buf.st_mtime != gi->mtime && (buf.st_mtime + 60 < gi->last_mtime_check))
      {
        /* GeoIP Database file updated */
        if (gi->flags & (GEOIP_MEMORY_CACHE | GEOIP_MMAP_CACHE))
        {
          if (gi->flags & GEOIP_MMAP_CACHE)
          {
#ifdef USE_MMAP
            /* MMAP is only avail on UNIX */
            munmap (gi->cache, gi->size);
            gi->cache = NULL;
#endif
          }
          else
          {
            /* reload database into memory cache */
            gi->cache = realloc (gi->cache, buf.st_size);
            if (!gi->cache)
            {
              printf ("Out of memory when reloading %s\n", gi->file_path);
              return (-1);
            }
          }
        }
        /* refresh filehandle */
        fclose (gi->GeoIPDatabase);
        gi->GeoIPDatabase = fopen (gi->file_path, "rb");
        if (!gi->GeoIPDatabase)
        {
          printf ("Error Opening file %s when reloading\n", gi->file_path);
          return (-1);
        }
        gi->mtime = buf.st_mtime;
        gi->size = buf.st_size;

        if (gi->flags & GEOIP_MMAP_CACHE)
        {
#ifndef USE_MMAP
          printf ("GEOIP_MMAP_CACHE is not supported on this platform\n");
          gi->cache = 0;
          return (-1);
#else
          gi->cache = mmap (NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fileno (gi->GeoIPDatabase), 0);
          if (gi->cache == MAP_FAILED)
          {

            printf ("Error remapping file %s when reloading\n", gi->file_path);
            gi->cache = NULL;
            return (-1);
          }
#endif
        }
        else if (gi->flags & GEOIP_MEMORY_CACHE)
        {
          if (pread (fileno (gi->GeoIPDatabase), gi->cache, buf.st_size, 0) != (ssize_t) buf.st_size)
          {
            printf ("Error reading file %s when reloading\n", gi->file_path);
            return (-1);
          }
        }

        if (gi->databaseSegments)
        {
          free (gi->databaseSegments);
          gi->databaseSegments = NULL;
        }
        _setup_segments (gi);
        if (!gi->databaseSegments)
        {
          printf ("Error reading file %s -- corrupt\n", gi->file_path);
          return (-1);
        }
        if (gi->flags & GEOIP_INDEX_CACHE)
        {
          size_t sz = gi->databaseSegments[0] * (long)gi->record_length * 2;

          gi->index_cache = realloc (gi->index_cache, sz);
          if (gi->index_cache)
          {
            if (pread (fileno(gi->GeoIPDatabase), gi->index_cache, sz, 0) != (ssize_t)sz)
            {
              printf ("Error reading file %s where reloading\n", gi->file_path);
              return (-1);
            }
          }
        }
      }
    }
  }
  return (0);
}

#define ADDR_STR_LEN (8 * 4 + 7 + 1)

unsigned int _GeoIP_seek_record_v6 (GeoIP *gi, geoipv6_t ipnum)
{
  int           j, fno, depth;
  char          paddr[ADDR_STR_LEN];
  unsigned int  x, offset = 0;
  unsigned char stack_buffer[2 * MAX_RECORD_LENGTH];
  const unsigned char *p, *buf = (gi->cache == NULL) ? stack_buffer : NULL;

  fno = fileno (gi->GeoIPDatabase);
  _check_mtime (gi);
  if (GeoIP_teredo(gi))
    __GEOIP_PREPARE_TEREDO (&ipnum);

  for (depth = 127; depth >= 0; depth--)
  {
    off_t ofs;

    if (gi->cache == NULL && gi->index_cache == NULL)
    {
      /* read from disk */
      ofs = (long)gi->record_length * 2 * offset;
      pread (fno, stack_buffer, gi->record_length * 2, ofs);
    }
    else if (gi->index_cache == NULL)
    {
      /* simply point to record in memory */
      ofs = (long)gi->record_length * 2 * offset;
      buf = gi->cache + ofs;
    }
    else
    {
      ofs = (long)gi->record_length * 2 * offset;
      buf = gi->index_cache + ofs;
    }

    if (GEOIP_CHKBIT_V6 (depth, ipnum.s6_addr))
    {
      /* Take the right-hand branch */
      if (gi->record_length == 3)
      {
        /* Most common case is completely unrolled and uses constants. */
        x = (buf[3] << (0 * 8)) + (buf[3 + 1] << (1 * 8)) + (buf[3 + 2] << (2 * 8));
      }
      else
      {
        /* General case */
        j = gi->record_length;
        p = &buf[2 * j];
        x = 0;
        do
        {
          x <<= 8;
          x += *(--p);
        }
        while (--j);
      }
    }
    else
    {
      /* Take the left-hand branch */
      if (gi->record_length == 3)
      {
        /* Most common case is completely unrolled and uses constants. */
        x = (buf[0] << (0 * 8)) + (buf[1] << (1 * 8)) + (buf[2] << (2 * 8));
      }
      else
      {
        /* General case */
        j = gi->record_length;
        p = &buf[1 * j];
        x = 0;
        do
        {
          x <<= 8;
          x += *(--p);
        }
        while (--j);
      }
    }

    if (x >= gi->databaseSegments[0])
    {
      gi->netmask = 128 - depth;
      return (x);
    }
    offset = x;
  }

  /* shouldn't reach here */
  _GeoIP_inet_ntop (AF_INET6, &ipnum.s6_addr[0], paddr, ADDR_STR_LEN);
  printf ("Error Traversing Database for ipnum = %s - Perhaps database is corrupt?\n", paddr);
  return (0);
}

geoipv6_t _GeoIP_addr_to_num_v6 (const char *addr)
{
  geoipv6_t ipnum;

  if (_GeoIP_inet_pton(AF_INET6, addr, &ipnum.s6_addr[0]) == 1)
     return (ipnum);
  return (IPV6_NULL);
}

unsigned int _GeoIP_seek_record (GeoIP *gi, unsigned long ipnum)
{
  unsigned char stack_buffer[2 * MAX_RECORD_LENGTH];
  unsigned int x, offset = 0;
  const unsigned char *p, *buf = (gi->cache == NULL) ? stack_buffer : NULL;
  int          depth, j, fno = fileno (gi->GeoIPDatabase);

  _check_mtime (gi);
  for (depth = 31; depth >= 0; depth--)
  {
    if (gi->cache == NULL && gi->index_cache == NULL)
    {
      /* read from disk */
      pread (fno, stack_buffer, gi->record_length * 2, gi->record_length * 2 * offset);
    }
    else if (gi->index_cache == NULL)
    {
      /* simply point to record in memory */
      buf = gi->cache + (long) gi->record_length * 2 * offset;
    }
    else
    {
      buf = gi->index_cache + (long) gi->record_length * 2 * offset;
    }

    if (ipnum & (1 << depth))
    {
      /* Take the right-hand branch */
      if (gi->record_length == 3)
      {
        /* Most common case is completely unrolled and uses constants. */
        x = (buf[3 * 1 + 0] << (0 * 8)) + (buf[3 * 1 + 1] << (1 * 8)) + (buf[3 * 1 + 2] << (2 * 8));
      }
      else
      {
        /* General case */
        j = gi->record_length;
        p = &buf[2 * j];
        x = 0;
        do
        {
          x <<= 8;
          x += *(--p);
        }
        while (--j);
      }
    }
    else
    {
      /* Take the left-hand branch */
      if (gi->record_length == 3)
      {
        /* Most common case is completely unrolled and uses constants. */
        x = (buf[3 * 0 + 0] << (0 * 8)) + (buf[3 * 0 + 1] << (1 * 8)) + (buf[3 * 0 + 2] << (2 * 8));
      }
      else
      {
        /* General case */
        j = gi->record_length;
        p = &buf[1 * j];
        x = 0;
        do
        {
          x <<= 8;
          x += *(--p);
        }
        while (--j);
      }
    }

    if (x >= gi->databaseSegments[0])
    {
      gi->netmask = 32 - depth;
      return x;
    }
    offset = x;
  }
  /* shouldn't reach here */
  printf ("Error Traversing Database for ipnum = %lu - Perhaps database is corrupt?\n", ipnum);
  return (0);
}

unsigned long GeoIP_addr_to_num (const char *addr)
{
  unsigned int c, octet, t;
  unsigned long ipnum;
  int i = 3;

  octet = ipnum = 0;
  while ((c = *addr++) != 0)
  {
    if (c == '.')
    {
      if (octet > 255)
         return (0);
      ipnum <<= 8;
      ipnum += octet;
      i--;
      octet = 0;
    }
    else
    {
      t = octet;
      octet <<= 3;
      octet += t;
      octet += t;
      c -= '0';
      if (c > 9)
         return (0);
      octet += c;
    }
  }
  if ((octet > 255) || (i != 0))
     return (0);
  ipnum <<= 8;
  return (ipnum + octet);
}

GeoIP *GeoIP_open_type (int type, int flags)
{
  GeoIP *gi;
  const char *filePath;

  if (type < 0 || type >= NUM_DB_TYPES)
  {
    printf ("Invalid database type %d\n", type);
    return (NULL);
  }
  _GeoIP_setup_dbfilename ();
  filePath = GeoIPDBFileName[type];
  if (filePath == NULL)
  {
    printf ("Invalid database type %d\n", type);
    return (NULL);
  }
  gi = GeoIP_open (filePath, flags);
  return (gi);
}

GeoIP *GeoIP_new (int flags)
{
  GeoIP *gi;

  _GeoIP_setup_dbfilename ();
  gi = GeoIP_open (GeoIPDBFileName[GEOIP_COUNTRY_EDITION], flags);
  return (gi);
}

GeoIP *GeoIP_open (const char *filename, int flags)
{
  struct stat buf;
  GeoIP *gi = malloc (sizeof (*gi));

  if (gi == NULL)
     return (NULL);

  gi->file_path = strdup (filename);
  if (gi->file_path == NULL)
  {
    free (gi);
    return (NULL);
  }
  gi->GeoIPDatabase = fopen (filename, "rb");
  if (gi->GeoIPDatabase == NULL)
  {
    printf ("Error Opening file %s\n", filename);
    free (gi->file_path);
    free (gi);
    return (NULL);
  }

  if (flags & (GEOIP_MEMORY_CACHE | GEOIP_MMAP_CACHE))
  {
    if (fstat (fileno (gi->GeoIPDatabase), &buf) == -1)
    {
      printf ("Error stating file %s\n", filename);
      free (gi->file_path);
      free (gi);
      return (NULL);
    }
    gi->mtime = buf.st_mtime;
    gi->size = buf.st_size;

    /* MMAP added my Peter Shipley */
    if (flags & GEOIP_MMAP_CACHE)
    {
#ifdef USE_MMAP
      gi->cache = mmap (NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fileno (gi->GeoIPDatabase), 0);
      if (gi->cache == MAP_FAILED)
      {
        printf ("Error mmaping file %s\n", filename);
        free (gi->file_path);
        free (gi);
        return (NULL);
      }
#endif
    }
    else
    {
      gi->cache = malloc (buf.st_size);
      if (gi->cache)
      {
        if (pread (fileno (gi->GeoIPDatabase), gi->cache, buf.st_size, 0) != (ssize_t) buf.st_size)
        {
          printf ("Error reading file %s\n", filename);
          free (gi->cache);
          free (gi->file_path);
          free (gi);
          return (NULL);
        }
      }
    }
  }
  else
  {
    if (flags & GEOIP_CHECK_CACHE)
    {
      if (fstat (fileno (gi->GeoIPDatabase), &buf) == -1)
      {
        printf ("Error stating file %s\n", filename);
        free (gi->file_path);
        free (gi);
        return (NULL);
      }
      gi->mtime = buf.st_mtime;
    }
    gi->cache = NULL;
  }

  gi->flags = flags;
  gi->charset = GEOIP_CHARSET_ISO_8859_1;
  gi->ext_flags = 1U << GEOIP_TEREDO_BIT;
  _setup_segments (gi);

  if (flags & GEOIP_INDEX_CACHE)
  {
    gi->index_cache = malloc (gi->databaseSegments[0] * (long) gi->record_length * 2);
    if (gi->index_cache)
    {
      off_t ofs = gi->databaseSegments[0] * (long) gi->record_length * 2;

      if (pread(fileno(gi->GeoIPDatabase), gi->index_cache, ofs, 0) != (size_t)ofs)
      {
        printf ("Error reading file %s\n", filename);
        free (gi->databaseSegments);
        free (gi->index_cache);
        free (gi);
        return (NULL);
      }
    }
  }
  else
  {
    gi->index_cache = NULL;
  }
  return (gi);
}

void GeoIP_delete (GeoIP *gi)
{
  if (gi == NULL)
     return;

  if (gi->GeoIPDatabase)
     fclose (gi->GeoIPDatabase);
  if (gi->cache)
  {
    if (gi->flags & GEOIP_MMAP_CACHE)
    {
#ifdef USE_MMAP
      munmap (gi->cache, gi->size);
#endif
    }
    else
      free (gi->cache);
    gi->cache = NULL;
  }
  if (gi->index_cache)
     free (gi->index_cache);
  if (gi->file_path)
     free (gi->file_path);
  if (gi->databaseSegments)
     free (gi->databaseSegments);
  free (gi);
}

#if defined(NOT_USED_IN_WATT32_TRACERT)

const char *GeoIP_country_code_by_name_v6 (GeoIP *gi, const char *name)
{
  int country_id = GeoIP_id_by_name_v6 (gi, name);

  return (country_id > 0) ? GeoIP_country_code[country_id] : NULL;
}

const char *GeoIP_country_code_by_name (GeoIP *gi, const char *name)
{
  int country_id = GeoIP_id_by_name (gi, name);

  return (country_id > 0) ? GeoIP_country_code[country_id] : NULL;
}

const char *GeoIP_country_code3_by_name_v6 (GeoIP *gi, const char *name)
{
  int country_id = GeoIP_id_by_name_v6 (gi, name);

  return (country_id > 0) ? GeoIP_country_code3[country_id] : NULL;
}

const char *GeoIP_country_code3_by_name (GeoIP *gi, const char *name)
{
  int country_id = GeoIP_id_by_name (gi, name);

  return (country_id > 0) ? GeoIP_country_code3[country_id] : NULL;
}

const char *GeoIP_country_name_by_name_v6 (GeoIP *gi, const char *name)
{
  int country_id = GeoIP_id_by_name_v6 (gi, name);

  return GeoIP_country_name_by_id (gi, country_id);
}

const char *GeoIP_country_name_by_name (GeoIP *gi, const char *name)
{
  int country_id = GeoIP_id_by_name (gi, name);

  return GeoIP_country_name_by_id (gi, country_id);
}

unsigned long _GeoIP_lookupaddress (const char *host)
{
  struct hostent  phe2;
  struct hostent *phe = &phe2;
  unsigned long   addr = inet_addr (host);
  int             result = 0;

  if (addr == INADDR_NONE)
  {
    /* Some systems do not support gethostbyname_r, such as Mac OS X */
    phe = gethostbyname (host);
    if (!phe || result != 0)
       return (0);
    addr = *(unsigned long*)phe->h_addr_list[0];
  }
  return ntohl (addr);
}

geoipv6_t _GeoIP_lookupaddress_v6 (const char *host)
{
  struct addrinfo hints, *aifirst;
  geoipv6_t ipnum;
  int       gaierr;

  memset (&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET6;

/*hints.ai_flags = AI_V4MAPPED; */
  hints.ai_socktype = SOCK_STREAM;

  gaierr = getaddrinfo (host, NULL, &hints, &aifirst);
  if (gaierr != 0)
     return (IPV6_NULL);

  memcpy (ipnum.s6_addr, ((struct sockaddr_in6*)aifirst->ai_addr)->sin6_addr.s6_addr, sizeof(geoipv6_t));
  freeaddrinfo (aifirst);
  return (ipnum);
}

int GeoIP_id_by_name (GeoIP *gi, const char *name)
{
  unsigned long ipnum;

  if (!name)
     return (0);

  if (gi->databaseType != GEOIP_LARGE_COUNTRY_EDITION &&
      gi->databaseType != GEOIP_COUNTRY_EDITION &&
      gi->databaseType != GEOIP_PROXY_EDITION &&
      gi->databaseType != GEOIP_NETSPEED_EDITION)
  {
    printf ("Invalid database type %s, expected %s\n", GeoIPDBDescription[(int) gi->databaseType],
            GeoIPDBDescription[GEOIP_COUNTRY_EDITION]);
    return (0);
  }
  ipnum = _GeoIP_lookupaddress (name);
  if (!ipnum)
     return (0);
  return _GeoIP_seek_record(gi, ipnum) - gi->databaseSegments[0];
}

int GeoIP_id_by_name_v6 (GeoIP *gi, const char *name)
{
  geoipv6_t ipnum;

  if (!name)
     return (0);

  if (gi->databaseType != GEOIP_LARGE_COUNTRY_EDITION_V6 &&
      gi->databaseType != GEOIP_COUNTRY_EDITION_V6)
  {
    printf ("Invalid database type %s, expected %s\n", GeoIPDBDescription[(int) gi->databaseType],
            GeoIPDBDescription[GEOIP_COUNTRY_EDITION_V6]);
    return (0);
  }
  ipnum = _GeoIP_lookupaddress_v6 (name);
  if (__GEOIP_V6_IS_NULL(ipnum))
     return (0);

  return _GeoIP_seek_record_v6 (gi, ipnum) - gi->databaseSegments[0];
}

const char *GeoIP_country_code_by_addr_v6 (GeoIP *gi, const char *addr)
{
  int country_id = GeoIP_id_by_addr_v6 (gi, addr);

  return (country_id > 0) ? GeoIP_country_code[country_id] : NULL;
}

const char *GeoIP_country_code_by_addr (GeoIP *gi, const char *addr)
{
  int country_id = GeoIP_id_by_addr (gi, addr);

  return (country_id > 0) ? GeoIP_country_code[country_id] : NULL;
}

const char *GeoIP_country_code3_by_addr_v6 (GeoIP *gi, const char *addr)
{
  int country_id = GeoIP_id_by_addr_v6 (gi, addr);
  return (country_id > 0) ? GeoIP_country_code3[country_id] : NULL;
}

const char *GeoIP_country_code3_by_addr (GeoIP *gi, const char *addr)
{
  int country_id = GeoIP_id_by_addr (gi, addr);

  return (country_id > 0) ? GeoIP_country_code3[country_id] : NULL;
}

const char *GeoIP_country_name_by_addr_v6 (GeoIP *gi, const char *addr)
{
  int country_id = GeoIP_id_by_addr_v6 (gi, addr);
  return GeoIP_country_name_by_id (gi, country_id);
}

const char *GeoIP_country_name_by_addr (GeoIP *gi, const char *addr)
{
  int country_id = GeoIP_id_by_addr (gi, addr);

  return GeoIP_country_name_by_id (gi, country_id);
}

const char *GeoIP_country_name_by_ipnum (GeoIP *gi, unsigned long ipnum)
{
  int country_id = GeoIP_id_by_ipnum (gi, ipnum);

  return GeoIP_country_name_by_id (gi, country_id);
}

const char *GeoIP_country_name_by_ipnum_v6 (GeoIP *gi, geoipv6_t ipnum)
{
  int country_id = GeoIP_id_by_ipnum_v6 (gi, ipnum);

  return GeoIP_country_name_by_id (gi, country_id);
}

const char *GeoIP_country_code_by_ipnum (GeoIP *gi, unsigned long ipnum)
{
  int country_id = GeoIP_id_by_ipnum (gi, ipnum);

  return (country_id > 0) ? GeoIP_country_code[country_id] : NULL;
}

const char *GeoIP_country_code_by_ipnum_v6 (GeoIP *gi, geoipv6_t ipnum)
{
  int country_id = GeoIP_id_by_ipnum_v6 (gi, ipnum);

  return (country_id > 0) ? GeoIP_country_code[country_id] : NULL;
}

const char *GeoIP_country_code3_by_ipnum (GeoIP *gi, unsigned long ipnum)
{
  int country_id = GeoIP_id_by_ipnum (gi, ipnum);

  return (country_id > 0) ? GeoIP_country_code3[country_id] : NULL;
}

const char *GeoIP_country_code3_by_ipnum_v6 (GeoIP *gi, geoipv6_t ipnum)
{
  int country_id = GeoIP_id_by_ipnum_v6 (gi, ipnum);

  return (country_id > 0) ? GeoIP_country_code3[country_id] : NULL;
}

int GeoIP_country_id_by_addr_v6 (GeoIP *gi, const char *addr)
{
  return GeoIP_id_by_addr_v6 (gi, addr);
}

int GeoIP_country_id_by_addr (GeoIP *gi, const char *addr)
{
  return GeoIP_id_by_addr (gi, addr);
}

int GeoIP_country_id_by_name_v6 (GeoIP *gi, const char *host)
{
  return GeoIP_id_by_name_v6 (gi, host);
}

int GeoIP_country_id_by_name (GeoIP *gi, const char *host)
{
  return GeoIP_id_by_name (gi, host);
}

int GeoIP_id_by_addr_v6 (GeoIP *gi, const char *addr)
{
  geoipv6_t ipnum;

  if (!addr)
     return (0);

  if (gi->databaseType != GEOIP_COUNTRY_EDITION_V6 &&
      gi->databaseType != GEOIP_LARGE_COUNTRY_EDITION_V6)
  {
    printf ("Invalid database type %s, expected %s\n",
            GeoIPDBDescription[(int) gi->databaseType], GeoIPDBDescription[GEOIP_COUNTRY_EDITION_V6]);
    return (0);
  }
  ipnum = _GeoIP_addr_to_num_v6 (addr);
  return _GeoIP_seek_record_v6 (gi, ipnum) - gi->databaseSegments[0];
}

int GeoIP_id_by_addr (GeoIP *gi, const char *addr)
{
  unsigned long ipnum;

  if (!addr)
     return (0);

  if (gi->databaseType != GEOIP_COUNTRY_EDITION &&
      gi->databaseType != GEOIP_LARGE_COUNTRY_EDITION &&
      gi->databaseType != GEOIP_PROXY_EDITION &&
      gi->databaseType != GEOIP_NETSPEED_EDITION)
  {
    printf ("Invalid database type %s, expected %s\n",
            GeoIPDBDescription[(int) gi->databaseType], GeoIPDBDescription[GEOIP_COUNTRY_EDITION]);
    return (0);
  }
  ipnum = GeoIP_addr_to_num (addr);
  return _GeoIP_seek_record (gi, ipnum) - gi->databaseSegments[0];
}
#endif     /* NOT_USED_IN_WATT32_TRACERT */

int GeoIP_id_by_ipnum_v6 (GeoIP *gi, geoipv6_t ipnum)
{
  int ret;

  /*     if (ipnum == 0) {
   * return (0);
   * }
   */
  if (gi->databaseType != GEOIP_COUNTRY_EDITION_V6 && gi->databaseType != GEOIP_LARGE_COUNTRY_EDITION_V6)
  {
    printf ("Invalid database type %s, expected %s\n",
            GeoIPDBDescription[(int) gi->databaseType], GeoIPDBDescription[GEOIP_COUNTRY_EDITION_V6]);
    return (0);
  }
  ret = _GeoIP_seek_record_v6 (gi, ipnum) - gi->databaseSegments[0];
  return ret;
}


int GeoIP_id_by_ipnum (GeoIP *gi, unsigned long ipnum)
{
  int ret;

  if (ipnum == 0)
  {
    return (0);
  }
  if (gi->databaseType != GEOIP_COUNTRY_EDITION &&
      gi->databaseType != GEOIP_LARGE_COUNTRY_EDITION &&
      gi->databaseType != GEOIP_PROXY_EDITION && gi->databaseType != GEOIP_NETSPEED_EDITION)
  {
    printf ("Invalid database type %s, expected %s\n",
            GeoIPDBDescription[(int) gi->databaseType], GeoIPDBDescription[GEOIP_COUNTRY_EDITION]);
    return (0);
  }
  ret = _GeoIP_seek_record (gi, ipnum) - gi->databaseSegments[0];
  return ret;
}

char *GeoIP_database_info (GeoIP *gi)
{
  int i;
  unsigned char buf[3];
  char *retval;
  int hasStructureInfo = 0;
  int fno = fileno (gi->GeoIPDatabase);

  if (gi == NULL)
    return (NULL);

  _check_mtime (gi);
  lseek (fno, -3l, SEEK_END);

  /* first get past the database structure information */
  for (i = 0; i < STRUCTURE_INFO_MAX_SIZE; i++)
  {
    read (fno, buf, 3);
    if (buf[0] == 255 && buf[1] == 255 && buf[2] == 255)
    {
      hasStructureInfo = 1;
      break;
    }
    lseek (fno, -4l, SEEK_CUR);
  }
  if (hasStructureInfo == 1)
  {
    lseek (fno, -6l, SEEK_CUR);
  }
  else
  {
    /* no structure info, must be pre Sep 2002 database, go back to end */
    lseek (fno, -3l, SEEK_END);
  }

  for (i = 0; i < DATABASE_INFO_MAX_SIZE; i++)
  {
    read (fno, buf, 3);
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0)
    {
      retval = malloc (i + 1);
      if (retval == NULL)
      {
        return (NULL);
      }
      read (fno, retval, i);
      retval[i] = '\0';
      return retval;
    }
    lseek (fno, -4l, SEEK_CUR);
  }
  return (NULL);
}

#if defined(NOT_USED_IN_WATT32_TRACERT)
/* GeoIP Region Edition functions */

void GeoIP_assign_region_by_inetaddr (GeoIP *gi, unsigned long inetaddr, GeoIPRegion * region)
{
  unsigned int seek_region;

  /* This also writes in the terminating NULs (if you decide to
   * keep them) and clear any fields that are not set. */
  memset (region, 0, sizeof (GeoIPRegion));

  seek_region = _GeoIP_seek_record (gi, ntohl (inetaddr));

  if (gi->databaseType == GEOIP_REGION_EDITION_REV0)
  {
    /* Region Edition, pre June 2003 */
    seek_region -= STATE_BEGIN_REV0;
    if (seek_region >= 1000)
    {
      region->country_code[0] = 'U';
      region->country_code[1] = 'S';
      region->region[0] = (char) ((seek_region - 1000) / 26 + 65);
      region->region[1] = (char) ((seek_region - 1000) % 26 + 65);
    }
    else
    {
      memcpy (region->country_code, GeoIP_country_code[seek_region], 2);
    }
  }
  else if (gi->databaseType == GEOIP_REGION_EDITION_REV1)
  {
    /* Region Edition, post June 2003 */
    seek_region -= STATE_BEGIN_REV1;
    if (seek_region < US_OFFSET)
    {
      /* Unknown */
      /* we don't need to do anything here b/c we memset region to 0 */
    }
    else if (seek_region < CANADA_OFFSET)
    {
      /* USA State */
      region->country_code[0] = 'U';
      region->country_code[1] = 'S';
      region->region[0] = (char) ((seek_region - US_OFFSET) / 26 + 65);
      region->region[1] = (char) ((seek_region - US_OFFSET) % 26 + 65);
    }
    else if (seek_region < WORLD_OFFSET)
    {
      /* Canada Province */
      region->country_code[0] = 'C';
      region->country_code[1] = 'A';
      region->region[0] = (char) ((seek_region - CANADA_OFFSET) / 26 + 65);
      region->region[1] = (char) ((seek_region - CANADA_OFFSET) % 26 + 65);
    }
    else
    {
      /* Not US or Canada */
      memcpy (region->country_code, GeoIP_country_code[(seek_region - WORLD_OFFSET) / FIPS_RANGE], 2);
    }
  }
}

void GeoIP_assign_region_by_inetaddr_v6 (GeoIP *gi, geoipv6_t inetaddr, GeoIPRegion * region)
{
  unsigned int seek_region;

  /* This also writes in the terminating NULs (if you decide to
   * keep them) and clear any fields that are not set. */
  memset (region, 0, sizeof (GeoIPRegion));

  seek_region = _GeoIP_seek_record_v6 (gi, inetaddr);

  if (gi->databaseType == GEOIP_REGION_EDITION_REV0)
  {
    /* Region Edition, pre June 2003 */
    seek_region -= STATE_BEGIN_REV0;
    if (seek_region >= 1000)
    {
      region->country_code[0] = 'U';
      region->country_code[1] = 'S';
      region->region[0] = (char) ((seek_region - 1000) / 26 + 65);
      region->region[1] = (char) ((seek_region - 1000) % 26 + 65);
    }
    else
    {
      memcpy (region->country_code, GeoIP_country_code[seek_region], 2);
    }
  }
  else if (gi->databaseType == GEOIP_REGION_EDITION_REV1)
  {
    /* Region Edition, post June 2003 */
    seek_region -= STATE_BEGIN_REV1;
    if (seek_region < US_OFFSET)
    {
      /* Unknown */
      /* we don't need to do anything here b/c we memset region to 0 */
    }
    else if (seek_region < CANADA_OFFSET)
    {
      /* USA State */
      region->country_code[0] = 'U';
      region->country_code[1] = 'S';
      region->region[0] = (char) ((seek_region - US_OFFSET) / 26 + 65);
      region->region[1] = (char) ((seek_region - US_OFFSET) % 26 + 65);
    }
    else if (seek_region < WORLD_OFFSET)
    {
      /* Canada Province */
      region->country_code[0] = 'C';
      region->country_code[1] = 'A';
      region->region[0] = (char) ((seek_region - CANADA_OFFSET) / 26 + 65);
      region->region[1] = (char) ((seek_region - CANADA_OFFSET) % 26 + 65);
    }
    else
    {
      /* Not US or Canada */
      memcpy (region->country_code, GeoIP_country_code[(seek_region - WORLD_OFFSET) / FIPS_RANGE], 2);
    }
  }
}

static GeoIPRegion *_get_region (GeoIP *gi, unsigned long ipnum)
{
  GeoIPRegion *region;

  region = malloc (sizeof (*region));
  if (region)
  {
    GeoIP_assign_region_by_inetaddr (gi, htonl (ipnum), region);
  }
  return region;
}

static GeoIPRegion *_get_region_v6 (GeoIP *gi, geoipv6_t ipnum)
{
  GeoIPRegion *region;

  region = malloc (sizeof (*region));
  if (region)
  {
    GeoIP_assign_region_by_inetaddr_v6 (gi, ipnum, region);
  }
  return region;
}

GeoIPRegion *GeoIP_region_by_addr (GeoIP *gi, const char *addr)
{
  unsigned long ipnum;

  if (addr == NULL)
  {
    return (0);
  }
  if (gi->databaseType != GEOIP_REGION_EDITION_REV0 && gi->databaseType != GEOIP_REGION_EDITION_REV1)
  {
    printf ("Invalid database type %s, expected %s\n", GeoIPDBDescription[(int) gi->databaseType],
            GeoIPDBDescription[GEOIP_REGION_EDITION_REV1]);
    return (0);
  }
  ipnum = GeoIP_addr_to_num (addr);
  return _get_region (gi, ipnum);
}

GeoIPRegion *GeoIP_region_by_addr_v6 (GeoIP *gi, const char *addr)
{
  geoipv6_t ipnum;

  if (addr == NULL)
  {
    return (0);
  }
  if (gi->databaseType != GEOIP_REGION_EDITION_REV0 && gi->databaseType != GEOIP_REGION_EDITION_REV1)
  {
    printf ("Invalid database type %s, expected %s\n", GeoIPDBDescription[(int) gi->databaseType],
            GeoIPDBDescription[GEOIP_REGION_EDITION_REV1]);
    return (0);
  }
  ipnum = _GeoIP_addr_to_num_v6 (addr);
  return _get_region_v6 (gi, ipnum);
}

GeoIPRegion *GeoIP_region_by_name (GeoIP *gi, const char *name)
{
  unsigned long ipnum;

  if (name == NULL)
  {
    return (0);
  }
  if (gi->databaseType != GEOIP_REGION_EDITION_REV0 && gi->databaseType != GEOIP_REGION_EDITION_REV1)
  {
    printf ("Invalid database type %s, expected %s\n", GeoIPDBDescription[(int) gi->databaseType],
            GeoIPDBDescription[GEOIP_REGION_EDITION_REV1]);
    return (0);
  }
  if (!(ipnum = _GeoIP_lookupaddress (name)))
    return (0);
  return _get_region (gi, ipnum);
}

GeoIPRegion *GeoIP_region_by_name_v6 (GeoIP *gi, const char *name)
{
  geoipv6_t ipnum;

  if (name == NULL)
  {
    return (0);
  }
  if (gi->databaseType != GEOIP_REGION_EDITION_REV0 && gi->databaseType != GEOIP_REGION_EDITION_REV1)
  {
    printf ("Invalid database type %s, expected %s\n", GeoIPDBDescription[(int) gi->databaseType],
            GeoIPDBDescription[GEOIP_REGION_EDITION_REV1]);
    return (0);
  }

  ipnum = _GeoIP_lookupaddress_v6 (name);
  if (__GEOIP_V6_IS_NULL(ipnum))
     return (0);
  return _get_region_v6 (gi, ipnum);
}

GeoIPRegion *GeoIP_region_by_ipnum (GeoIP *gi, unsigned long ipnum)
{
  if (gi->databaseType != GEOIP_REGION_EDITION_REV0 && gi->databaseType != GEOIP_REGION_EDITION_REV1)
  {
    printf ("Invalid database type %s, expected %s\n", GeoIPDBDescription[(int) gi->databaseType],
            GeoIPDBDescription[GEOIP_REGION_EDITION_REV1]);
    return (0);
  }
  return _get_region (gi, ipnum);
}

GeoIPRegion *GeoIP_region_by_ipnum_v6 (GeoIP *gi, geoipv6_t ipnum)
{
  if (gi->databaseType != GEOIP_REGION_EDITION_REV0 && gi->databaseType != GEOIP_REGION_EDITION_REV1)
  {
    printf ("Invalid database type %s, expected %s\n", GeoIPDBDescription[(int) gi->databaseType],
            GeoIPDBDescription[GEOIP_REGION_EDITION_REV1]);
    return (0);
  }
  return _get_region_v6 (gi, ipnum);
}

void GeoIPRegion_delete (GeoIPRegion * gir)
{
  free (gir);
}
#endif  /* NOT_USED_IN_WATT32_TRACERT */

/*
 * GeoIP Organization, ISP and AS Number Edition private method
 */
static char *_get_name (GeoIP *gi, unsigned long ipnum)
{
  int   seek_org, record_pointer;
  char  buf[MAX_ORG_RECORD_LENGTH];
  char *org_buf, *buf_pointer;

  if (gi->databaseType != GEOIP_ORG_EDITION &&
      gi->databaseType != GEOIP_ISP_EDITION &&
      gi->databaseType != GEOIP_DOMAIN_EDITION &&
      gi->databaseType != GEOIP_ASNUM_EDITION &&
      gi->databaseType != GEOIP_NETSPEED_EDITION_REV1 &&
      gi->databaseType != GEOIP_USERTYPE_EDITION &&
      gi->databaseType != GEOIP_REGISTRAR_EDITION && gi->databaseType != GEOIP_LOCATIONA_EDITION)
  {
    printf ("Invalid database type %s, expected %s\n",
            GeoIPDBDescription[(int) gi->databaseType], GeoIPDBDescription[GEOIP_ORG_EDITION]);
    return (NULL);
  }

  seek_org = _GeoIP_seek_record (gi, ipnum);
  if (seek_org == gi->databaseSegments[0])
    return (NULL);

  record_pointer = seek_org + (2 * gi->record_length - 1) * gi->databaseSegments[0];

  if (gi->cache == NULL)
  {
    pread (fileno (gi->GeoIPDatabase), buf, MAX_ORG_RECORD_LENGTH, record_pointer);
    if (gi->charset == GEOIP_CHARSET_UTF8)
    {
      org_buf = _GeoIP_iso_8859_1__utf8 ((const char*)buf);
    }
    else
    {
      org_buf = strdup (buf);
    }
  }
  else
  {
    buf_pointer = (char *) (gi->cache + (long) record_pointer);
    if (gi->charset == GEOIP_CHARSET_UTF8)
    {
      org_buf = _GeoIP_iso_8859_1__utf8 ((const char*)buf_pointer);
    }
    else
    {
      org_buf = strdup (buf_pointer);
    }
  }
  return org_buf;
}

char *_get_name_v6 (GeoIP *gi, geoipv6_t ipnum)
{
  int   seek_org, record_pointer;
  char  buf [MAX_ORG_RECORD_LENGTH];
  char *org_buf, *buf_pointer;

  if (gi->databaseType != GEOIP_ORG_EDITION_V6 &&
      gi->databaseType != GEOIP_ISP_EDITION_V6 &&
      gi->databaseType != GEOIP_DOMAIN_EDITION_V6 &&
      gi->databaseType != GEOIP_ASNUM_EDITION_V6 &&
      gi->databaseType != GEOIP_NETSPEED_EDITION_REV1_V6 &&
      gi->databaseType != GEOIP_USERTYPE_EDITION_V6 &&
      gi->databaseType != GEOIP_REGISTRAR_EDITION_V6 && gi->databaseType != GEOIP_LOCATIONA_EDITION)
  {
    printf ("Invalid database type %s, expected %s\n", GeoIPDBDescription[(int) gi->databaseType],
            GeoIPDBDescription[GEOIP_ORG_EDITION]);
    return (NULL);
  }

  seek_org = _GeoIP_seek_record_v6 (gi, ipnum);
  if (seek_org == gi->databaseSegments[0])
    return (NULL);

  record_pointer = seek_org + (2 * gi->record_length - 1) * gi->databaseSegments[0];

  if (gi->cache == NULL)
  {
    pread (fileno (gi->GeoIPDatabase), buf, MAX_ORG_RECORD_LENGTH, record_pointer);
    if (gi->charset == GEOIP_CHARSET_UTF8)
    {
      org_buf = _GeoIP_iso_8859_1__utf8 ((const char*)buf);
    }
    else
    {
      org_buf = strdup (buf);
    }
  }
  else
  {
    buf_pointer = (char *) (gi->cache + (long) record_pointer);
    if (gi->charset == GEOIP_CHARSET_UTF8)
    {
      org_buf = _GeoIP_iso_8859_1__utf8 ((const char*)buf_pointer);
    }
    else
    {
      org_buf = strdup (buf_pointer);
    }
  }
  return org_buf;
}

char *GeoIP_num_to_addr (unsigned long ipnum)
{
  char *ret_str;
  char *cur_str;
  int octet[4];
  int num_chars_written, i;

  ret_str = malloc (16);
  cur_str = ret_str;

  for (i = 0; i < 4; i++)
  {
    octet[3 - i] = ipnum % 256;
    ipnum >>= 8;
  }

  for (i = 0; i < 4; i++)
  {
    num_chars_written = sprintf (cur_str, "%d", octet[i]);
    cur_str += num_chars_written;

    if (i < 3)
    {
      cur_str[0] = '.';
      cur_str++;
    }
  }

  return ret_str;
}

char **GeoIP_range_by_ip (GeoIP *gi, const char *addr)
{
  unsigned long ipnum;
  unsigned long left_seek;
  unsigned long right_seek;
  unsigned long mask;
  int orig_netmask;
  int target_value;
  char **ret;

  if (addr == NULL)
  {
    return (0);
  }

  ret = malloc (sizeof (char *) * 2);

  ipnum = GeoIP_addr_to_num (addr);
  target_value = _GeoIP_seek_record (gi, ipnum);
  orig_netmask = GeoIP_last_netmask (gi);
  mask = 0xffffffff << (32 - orig_netmask);
  left_seek = ipnum & mask;
  right_seek = left_seek + (0xffffffff & ~mask);

  while (left_seek != 0 && target_value == _GeoIP_seek_record (gi, left_seek - 1))
  {

    /* Go to beginning of netblock defined by netmask */
    mask = 0xffffffff << (32 - GeoIP_last_netmask (gi));
    left_seek = (left_seek - 1) & mask;
  }
  ret[0] = GeoIP_num_to_addr (left_seek);

  while (right_seek != 0xffffffff && target_value == _GeoIP_seek_record (gi, right_seek + 1))
  {

    /* Go to end of netblock defined by netmask */
    mask = 0xffffffff << (32 - GeoIP_last_netmask (gi));
    right_seek = (right_seek + 1) & mask;
    right_seek += 0xffffffff & ~mask;
  }
  ret[1] = GeoIP_num_to_addr (right_seek);

  gi->netmask = orig_netmask;

  return ret;
}

void GeoIP_range_by_ip_delete (char **ptr)
{
  if (ptr)
  {
    if (ptr[0])
      free (ptr[0]);
    if (ptr[1])
      free (ptr[1]);
    free (ptr);
  }
}

char *GeoIP_name_by_ipnum (GeoIP *gi, unsigned long ipnum)
{
  return _get_name (gi, ipnum);
}

char *GeoIP_name_by_ipnum_v6 (GeoIP *gi, geoipv6_t ipnum)
{
  return _get_name_v6 (gi, ipnum);
}

char *GeoIP_name_by_addr (GeoIP *gi, const char *addr)
{
  unsigned long ipnum;

  if (addr == NULL)
  {
    return (0);
  }
  ipnum = GeoIP_addr_to_num (addr);
  return _get_name (gi, ipnum);
}

char *GeoIP_name_by_addr_v6 (GeoIP *gi, const char *addr)
{
  geoipv6_t ipnum;

  if (addr == NULL)
  {
    return (0);
  }
  ipnum = _GeoIP_addr_to_num_v6 (addr);
  return _get_name_v6 (gi, ipnum);
}

#if defined(NOT_USED_IN_WATT32_TRACERT)

char *GeoIP_name_by_name (GeoIP *gi, const char *name)
{
  unsigned long ipnum;

  if (!name)
     return (NULL);

  ipnum = _GeoIP_lookupaddress (name);
  if (ipnum == 0)
     return (0);
  return _get_name (gi, ipnum);
}

char *GeoIP_name_by_name_v6 (GeoIP *gi, const char *name)
{
  geoipv6_t ipnum;

  if (!name)
     return (NULL);

  ipnum = _GeoIP_lookupaddress_v6 (name);
  if (__GEOIP_V6_IS_NULL(ipnum))
     return (0);
  return _get_name_v6 (gi, ipnum);
}

char *GeoIP_org_by_ipnum (GeoIP *gi, unsigned long ipnum)
{
  return GeoIP_name_by_ipnum (gi, ipnum);
}

char *GeoIP_org_by_ipnum_v6 (GeoIP *gi, geoipv6_t ipnum)
{
  return GeoIP_name_by_ipnum_v6 (gi, ipnum);
}

char *GeoIP_org_by_addr (GeoIP *gi, const char *addr)
{
  return GeoIP_name_by_addr (gi, addr);
}

char *GeoIP_org_by_addr_v6 (GeoIP *gi, const char *addr)
{
  return GeoIP_name_by_addr_v6 (gi, addr);
}

char *GeoIP_org_by_name (GeoIP *gi, const char *name)
{
  return GeoIP_name_by_name (gi, name);
}

char *GeoIP_org_by_name_v6 (GeoIP *gi, const char *name)
{
  return GeoIP_name_by_name_v6 (gi, name);
}
#endif     /* NOT_USED_IN_WATT32_TRACERT */

unsigned char GeoIP_database_edition (GeoIP *gi)
{
  return (gi->databaseType);
}

int GeoIP_enable_teredo (GeoIP *gi, int true_false)
{
  unsigned int mask = (1U << GEOIP_TEREDO_BIT);
  int          b = (gi->ext_flags & mask) ? 1 : 0;

  gi->ext_flags &= ~mask;
  if (true_false)
     gi->ext_flags |= true_false;
  return b;
}

int GeoIP_teredo (GeoIP *gi)
{
  unsigned int mask = (1U << GEOIP_TEREDO_BIT);

  return (gi->ext_flags & mask) ? 1 : 0;
}

int GeoIP_charset (GeoIP *gi)
{
  return (gi->charset);
}

int GeoIP_set_charset (GeoIP *gi, int charset)
{
  int old_charset = gi->charset;

  gi->charset = charset;
  return (old_charset);
}

int GeoIP_last_netmask (GeoIP *gi)
{
  return (gi->netmask);
}

/*
 * return two letter country code
  */
const char *GeoIP_code_by_id (int id)
{
  if (id < 0 || id >= (int) num_GeoIP_countries)
     return (NULL);
  return GeoIP_country_code[id];
}

#if defined(NOT_USED_IN_WATT32_TRACERT)
/*
 * return three letter country code
 */
const char *GeoIP_code3_by_id (int id)
{
  if (id < 0 || id >= (int) num_GeoIP_countries)
     return (NULL);
  return GeoIP_country_code3[id];
}
#endif

/*
 * return full name of country in utf8 or iso-8859-1
 */
const char *GeoIP_country_name_by_id (GeoIP *gi, int id)
{
  /* return NULL also even for index 0 for backward compatibility */
  if (id <= 0 || id >= (int) num_GeoIP_countries)
     return (NULL);
  return ((gi->charset == GEOIP_CHARSET_UTF8) ? GeoIP_utf8_country_name[id] : GeoIP_country_name[id]);
}

/*
 * return full name of country in iso-8859-1
 */
const char *GeoIP_name_by_id (int id)
{
  if (id < 0 || id >= (int)num_GeoIP_countries)
     return (NULL);
  return GeoIP_country_name[id];
}

/*
 * return continent of country
 */
const char *GeoIP_continent_by_id (int id)
{
  if (id < 0 || id >= (int) num_GeoIP_countries)
     return (NULL);
  return GeoIP_country_continent[id];
}

/*
 * return id by country code
 */
int GeoIP_id_by_code (const char *country)
{
  unsigned i;

  for (i = 0; i < num_GeoIP_countries; ++i)
  {
    if (!strcmp (country, GeoIP_country_code[i]))
       return i;
  }
  return (0);
}

unsigned GeoIP_num_countries (void)
{
  return (num_GeoIP_countries);
}

const char *GeoIP_lib_version (void)
{
#ifdef PACKAGE_VERSION
  return PACKAGE_VERSION;
#else
  return "<unknown>";
#endif
}

int GeoIP_cleanup (void)
{
  int i, result = 0;

  if (GeoIPDBFileName)
  {
    for (i = 0; i < NUM_DB_TYPES; i++)
    {
      if (GeoIPDBFileName[i])
         free (GeoIPDBFileName[i]);
    }
    free (GeoIPDBFileName);
    GeoIPDBFileName = NULL;
    result = 1;
  }
  return (result);
}
#endif   /* USE_GEOIP */
