/*
 * IP-address to country converter.
 *
 * Problem; you want to know where IP a.b.c.d is located.
 *
 * Use gethostbyname ("d.c.b.a.zz.countries.nerd.dk")
 * and get the CNAME (he->h_name). Result will be:
 *   CNAME = zz<CC>.countries.nerd.dk with address 127.0.x.y (ver 1) or
 *   CNAME = <a.b.c.d>.zz.countries.nerd.dk with address 127.0.x.y (ver 2)
 *
 * The 2 letter country code in <CC> and the ISO-3166 country
 * number in x.y (number = x*256 + y). Version 2 of the protocol is missing
 * the <CC> number.
 *
 * Ref: http://countries.nerd.dk/more.html
 *
 * Written by G. Vanem <gvanem@yahoo.no> 2003, 2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <tcp.h>

#if defined(__CYGWIN__)
  #define stricmp(s1, s2)   strcasecmp (s1, s2)
#endif

#if defined(__GNUC__)
  #pragma GCC diagnostic ignored  "-Wchar-subscripts"
  #pragma GCC diagnostic ignored  "-Wstrict-aliasing"
#endif

static const char *usage    = "country [-vh?] IPv4-address\n";
static const char *nerd_fmt = "%d.%d.%d.%d.zz.countries.nerd.dk";
static int         verbose  = 0;

#define TRACE(fmt) do {               \
                     if (verbose > 0) \
                        printf fmt ;  \
                   } while (0)

static int find_country_from_cname (const char *cname, struct in_addr addr);

static void Abort (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  exit (1);
}

int MS_CDECL main (int argc, char **argv)
{
  struct hostent *he;
  u_long host;
  u_char *addr;
  char   buf[50], *cname;
  int    ch;

  while ((ch = getopt(argc, argv, "vh?")) != EOF)
        switch (ch)
        {
          case 'v':
               verbose++;
               break;
          case 'h':
          case '?':
          default:
               Abort (usage);
        }

  argc -= optind;
  argv += optind;
  if (argc < 1)
     Abort (usage);

  if (verbose)
     dbug_init();

  sock_init();

  host = inet_addr (*argv);
  if (host == INADDR_NONE)
     Abort ("%s is not an IP-address\n", *argv);

  addr = (u_char*) &host;
  sprintf (buf, nerd_fmt, addr[3], addr[2], addr[1], addr[0]);
  TRACE (("Looking up %s...", buf));
  fflush (stdout);

  he = gethostbyname (buf);
  if (!he)
     Abort ("Failed to lookup %s\n", buf);

  TRACE (("\nFound address %s, name %s and\nalias `%s'\n",
          inet_ntoa(*(struct in_addr*)he->h_addr),
          he->h_name, he->h_aliases[0]));

  cname = he->h_name;  /* CNAME gets put here */
  if (!cname)
     Abort ("Failed to get CNAME for %s\n", buf);

  return find_country_from_cname (cname, *(struct in_addr*)he->h_addr);
}

struct search_list {
       int         country_number; /* ISO-3166 country number */
       char        short_name[3];  /* A2 short country code */
       const char *long_name;      /* normal country name */
     };

const struct search_list *list_lookup (int number, const struct search_list *list, int num)
{
  while (num > 0 && list->long_name)
  {
    if (list->country_number == number)
       return (list);
    num--;
    list++;
  }
  return (NULL);
}

/*
 * Ref: ftp://ftp.ripe.net/iso3166-countrycodes.txt
 */
static const struct search_list country_list[] = {
       {   4, "af", "Afghanistan"                          },
       { 248, "ax", "Åland Island"                         },
       {   8, "al", "Albania"                              },
       {  12, "dz", "Algeria"                              },
       {  16, "as", "American Samoa"                       },
       {  20, "ad", "Andorra"                              },
       {  24, "ao", "Angola"                               },
       { 660, "ai", "Anguilla"                             },
       {  10, "aq", "Antarctica"                           },
       {  28, "ag", "Antigua & Barbuda"                    },
       {  32, "ar", "Argentina"                            },
       {  51, "am", "Armenia"                              },
       { 533, "aw", "Aruba"                                },
       {  36, "au", "Australia"                            },
       {  40, "at", "Austria"                              },
       {  31, "az", "Azerbaijan"                           },
       {  44, "bs", "Bahamas"                              },
       {  48, "bh", "Bahrain"                              },
       {  50, "bd", "Bangladesh"                           },
       {  52, "bb", "Barbados"                             },
       { 112, "by", "Belarus"                              },
       {  56, "be", "Belgium"                              },
       {  84, "bz", "Belize"                               },
       { 204, "bj", "Benin"                                },
       {  60, "bm", "Bermuda"                              },
       {  64, "bt", "Bhutan"                               },
       {  68, "bo", "Bolivia"                              },
       {  70, "ba", "Bosnia & Herzegowina"                 },
       {  72, "bw", "Botswana"                             },
       {  74, "bv", "Bouvet Island"                        },
       {  76, "br", "Brazil"                               },
       {  86, "io", "British Indian Ocean Territory"       },
       {  96, "bn", "Brunei Darussalam"                    },
       { 100, "bg", "Bulgaria"                             },
       { 854, "bf", "Burkina Faso"                         },
       { 108, "bi", "Burundi"                              },
       { 116, "kh", "Cambodia"                             },
       { 120, "cm", "Cameroon"                             },
       { 124, "ca", "Canada"                               },
       { 132, "cv", "Cape Verde"                           },
       { 136, "ky", "Cayman Islands"                       },
       { 140, "cf", "Central African Republic"             },
       { 148, "td", "Chad"                                 },
       { 152, "cl", "Chile"                                },
       { 156, "cn", "China"                                },
       { 162, "cx", "Christmas Island"                     },
       { 166, "cc", "Cocos Islands"                        },
       { 170, "co", "Colombia"                             },
       { 174, "km", "Comoros"                              },
       { 178, "cg", "Congo"                                },
       { 180, "cd", "Congo"                                },
       { 184, "ck", "Cook Islands"                         },
       { 188, "cr", "Costa Rica"                           },
       { 384, "ci", "Cote d'Ivoire"                        },
       { 191, "hr", "Croatia"                              },
       { 192, "cu", "Cuba"                                 },
       { 196, "cy", "Cyprus"                               },
       { 203, "cz", "Czech Republic"                       },
       { 208, "dk", "Denmark"                              },
       { 262, "dj", "Djibouti"                             },
       { 212, "dm", "Dominica"                             },
       { 214, "do", "Dominican Republic"                   },
       { 218, "ec", "Ecuador"                              },
       { 818, "eg", "Egypt"                                },
       { 222, "sv", "El Salvador"                          },
       { 226, "gq", "Equatorial Guinea"                    },
       { 232, "er", "Eritrea"                              },
       { 233, "ee", "Estonia"                              },
       { 231, "et", "Ethiopia"                             },
       { 238, "fk", "Falkland Islands"                     },
       { 234, "fo", "Faroe Islands"                        },
       { 242, "fj", "Fiji"                                 },
       { 246, "fi", "Finland"                              },
       { 250, "fr", "France"                               },
       { 249, "fx", "France, Metropolitan"                 },
       { 254, "gf", "French Guiana"                        },
       { 258, "pf", "French Polynesia"                     },
       { 260, "tf", "French Southern Territories"          },
       { 266, "ga", "Gabon"                                },
       { 270, "gm", "Gambia"                               },
       { 268, "ge", "Georgia"                              },
       { 276, "de", "Germany"                              },
       { 288, "gh", "Ghana"                                },
       { 292, "gi", "Gibraltar"                            },
       { 300, "gr", "Greece"                               },
       { 304, "gl", "Greenland"                            },
       { 308, "gd", "Grenada"                              },
       { 312, "gp", "Guadeloupe"                           },
       { 316, "gu", "Guam"                                 },
       { 320, "gt", "Guatemala"                            },
       { 324, "gn", "Guinea"                               },
       { 624, "gw", "Guinea-Bissau"                        },
       { 328, "gy", "Guyana"                               },
       { 332, "ht", "Haiti"                                },
       { 334, "hm", "Heard & Mc Donald Islands"            },
       { 336, "va", "Vatican City"                         },
       { 340, "hn", "Honduras"                             },
       { 344, "hk", "Hong kong"                            },
       { 348, "hu", "Hungary"                              },
       { 352, "is", "Iceland"                              },
       { 356, "in", "India"                                },
       { 360, "id", "Indonesia"                            },
       { 364, "ir", "Iran"                                 },
       { 368, "iq", "Iraq"                                 },
       { 372, "ie", "Ireland"                              },
       { 376, "il", "Israel"                               },
       { 380, "it", "Italy"                                },
       { 388, "jm", "Jamaica"                              },
       { 392, "jp", "Japan"                                },
       { 400, "jo", "Jordan"                               },
       { 398, "kz", "Kazakhstan"                           },
       { 404, "ke", "Kenya"                                },
       { 296, "ki", "Kiribati"                             },
       { 408, "kp", "Korea (north)"                        },
       { 410, "kr", "Korea (south)"                        },
       { 414, "kw", "Kuwait"                               },
       { 417, "kg", "Kyrgyzstan"                           },
       { 418, "la", "Laos"                                 },
       { 428, "lv", "Latvia"                               },
       { 422, "lb", "Lebanon"                              },
       { 426, "ls", "Lesotho"                              },
       { 430, "lr", "Liberia"                              },
       { 434, "ly", "Libya"                                },
       { 438, "li", "Liechtenstein"                        },
       { 440, "lt", "Lithuania"                            },
       { 442, "lu", "Luxembourg"                           },
       { 446, "mo", "Macao"                                },
       { 807, "mk", "Macedonia"                            },
       { 450, "mg", "Madagascar"                           },
       { 454, "mw", "Malawi"                               },
       { 458, "my", "Malaysia"                             },
       { 462, "mv", "Maldives"                             },
       { 466, "ml", "Mali"                                 },
       { 470, "mt", "Malta"                                },
       { 584, "mh", "Marshall Islands"                     },
       { 474, "mq", "Martinique"                           },
       { 478, "mr", "Mauritania"                           },
       { 480, "mu", "Mauritius"                            },
       { 175, "yt", "Mayotte"                              },
       { 484, "mx", "Mexico"                               },
       { 583, "fm", "Micronesia"                           },
       { 498, "md", "Moldova"                              },
       { 492, "mc", "Monaco"                               },
       { 496, "mn", "Mongolia"                             },
       { 500, "ms", "Montserrat"                           },
       { 504, "ma", "Morocco"                              },
       { 508, "mz", "Mozambique"                           },
       { 104, "mm", "Myanmar"                              },
       { 516, "na", "Namibia"                              },
       { 520, "nr", "Nauru"                                },
       { 524, "np", "Nepal"                                },
       { 528, "nl", "Netherlands"                          },
       { 530, "an", "Netherlands Antilles"                 },
       { 540, "nc", "New Caledonia"                        },
       { 554, "nz", "New Zealand"                          },
       { 558, "ni", "Nicaragua"                            },
       { 562, "ne", "Niger"                                },
       { 566, "ng", "Nigeria"                              },
       { 570, "nu", "Niue"                                 },
       { 574, "nf", "Norfolk Island"                       },
       { 580, "mp", "Northern Mariana Islands"             },
       { 578, "no", "Norway"                               },
       { 512, "om", "Oman"                                 },
       { 586, "pk", "Pakistan"                             },
       { 585, "pw", "Palau"                                },
       { 275, "ps", "Palestinian Territory"                },
       { 591, "pa", "Panama"                               },
       { 598, "pg", "Papua New Guinea"                     },
       { 600, "py", "Paraguay"                             },
       { 604, "pe", "Peru"                                 },
       { 608, "ph", "Philippines"                          },
       { 612, "pn", "Pitcairn"                             },
       { 616, "pl", "Poland"                               },
       { 620, "pt", "Portugal"                             },
       { 630, "pr", "Puerto Rico"                          },
       { 634, "qa", "Qatar"                                },
       { 638, "re", "Reunion"                              },
       { 642, "ro", "Romania"                              },
       { 643, "ru", "Russia"                               },
       { 646, "rw", "Rwanda"                               },
       { 659, "kn", "Saint Kitts & Nevis"                  },
       { 662, "lc", "Saint Lucia"                          },
       { 670, "vc", "Saint Vincent"                        },
       { 882, "ws", "Samoa"                                },
       { 674, "sm", "San Marino"                           },
       { 678, "st", "Sao Tome & Principe"                  },
       { 682, "sa", "Saudi Arabia"                         },
       { 686, "sn", "Senegal"                              },
       { 891, "cs", "Serbia and Montenegro"                },
       { 690, "sc", "Seychelles"                           },
       { 694, "sl", "Sierra Leone"                         },
       { 702, "sg", "Singapore"                            },
       { 703, "sk", "Slovakia"                             },
       { 705, "si", "Slovenia"                             },
       {  90, "sb", "Solomon Islands"                      },
       { 706, "so", "Somalia"                              },
       { 710, "za", "South Africa"                         },
       { 239, "gs", "South Georgia"                        },
       { 724, "es", "Spain"                                },
       { 144, "lk", "Sri Lanka"                            },
       { 654, "sh", "St. Helena"                           },
       { 666, "pm", "St. Pierre & Miquelon"                },
       { 736, "sd", "Sudan"                                },
       { 740, "sr", "Suriname"                             },
       { 744, "sj", "Svalbard & Jan Mayen Islands"         },
       { 748, "sz", "Swaziland"                            },
       { 752, "se", "Sweden"                               },
       { 756, "ch", "Switzerland"                          },
       { 760, "sy", "Syrian Arab Republic"                 },
       { 626, "tl", "Timor-Leste"                          },
       { 158, "tw", "Taiwan"                               },
       { 762, "tj", "Tajikistan"                           },
       { 834, "tz", "Tanzania"                             },
       { 764, "th", "Thailand"                             },
       { 768, "tg", "Togo"                                 },
       { 772, "tk", "Tokelau"                              },
       { 776, "to", "Tonga"                                },
       { 780, "tt", "Trinidad & Tobago"                    },
       { 788, "tn", "Tunisia"                              },
       { 792, "tr", "Turkey"                               },
       { 795, "tm", "Turkmenistan"                         },
       { 796, "tc", "Turks & Caicos Islands"               },
       { 798, "tv", "Tuvalu"                               },
       { 800, "ug", "Uganda"                               },
       { 804, "ua", "Ukraine"                              },
       { 784, "ae", "United Arab Emirates"                 },
       { 826, "gb", "United Kingdom"                       },
       { 840, "us", "United States"                        },
       { 581, "um", "United States Minor Outlying Islands" },
       { 858, "uy", "Uruguay"                              },
       { 860, "uz", "Uzbekistan"                           },
       { 548, "vu", "Vanuatu"                              },
       { 862, "ve", "Venezuela"                            },
       { 704, "vn", "Vietnam"                              },
       {  92, "vg", "Virgin Islands (British)"             },
       { 850, "vi", "Virgin Islands (US)"                  },
       { 876, "wf", "Wallis & Futuna Islands"              },
       { 732, "eh", "Western Sahara"                       },
       { 887, "ye", "Yemen"                                },
       { 894, "zm", "Zambia"                               },
       { 716, "zw", "Zimbabwe"                             }
     };

/**
 * Check if start of 'str' is simply an IPv4 address.
 */
#define BYTE_OK(x) ((x) >= 0 && (x) <= 255)

static int _isaddr (char *str, char **end)
{
  int a0, a1, a2, a3, num, rc = 0, length = 0;

  if ((num = sscanf(str,"%3d.%3d.%3d.%3d%n",&a0,&a1,&a2,&a3,&length)) == 4 &&
      BYTE_OK(a0) && BYTE_OK(a1) && BYTE_OK(a2) && BYTE_OK(a3) &&
      length >= (3+4))
  {
    rc = 1;
    *end = str + length;
  }
  return (rc);
}

/*
 * E.g.
 *   version 1: CNAME = zzno.countries.nerd.dk with address 127.0.2.66
 *              yields ccode_A" = "no" and cnumber 578 (2.66).
 *   version 2: CNAME = <a.b.c.d>.zz.countries.nerd.dk with address 127.0.2.66
 *              yields cnumber 578 (2.66). ccode_A is "";
 */
static int find_country_from_cname (const char *cname, struct in_addr addr)
{
  const struct search_list *country;
  char  ccode_A2[3], *ccopy, *dot_4;
  int   cnumber, z0, z1, ver_1, ver_2;
  u_long         ip;

  ip = ntohl (addr.s_addr);
  z0 = tolower (cname[0]);
  z1 = tolower (cname[1]);
  ccopy = strdup (cname);

  ver_1 = (z0 == 'z' && z1 == 'z' && !stricmp(cname+4,".countries.nerd.dk"));
  ver_2 = (_isaddr(ccopy,&dot_4) && !stricmp(dot_4,".zz.countries.nerd.dk"));

  if (ver_1)
  {
    const char *dot = strchr (cname, '.');
    if ((z0 != 'z' && z1 != 'z') || dot != cname+4)
    {
      printf ("Unexpected CNAME %s (ver_1)\n", cname);
      return (-1);
    }
  }
  else if (ver_2)
  {
    z0 = tolower (dot_4[1]);
    z1 = tolower (dot_4[2]);
    if (z0 != 'z' && z1 != 'z')
    {
      printf ("Unexpected CNAME %s (ver_2)\n", cname);
      return (-1);
    }
  }
  else
  {
    printf ("Unexpected CNAME %s (ver?)\n", cname);
    return (-1);
  }

  if (ver_1)
  {
    ccode_A2[0] = tolower (cname[2]);
    ccode_A2[1] = tolower (cname[3]);
    ccode_A2[2] = '\0';
  }
  else
    ccode_A2[0] = '\0';

  cnumber = ip & 0xFFFF;

  TRACE (("Found country-code `%s', number %d\n",
          ver_1 ? ccode_A2 : "<n/a>", cnumber));

  country = list_lookup (cnumber, country_list,
                         sizeof(country_list) / sizeof(country_list[0]));
  if (!country)
  {
    printf ("Name for country-number %d not found.\n", cnumber);
    free (ccopy);
    return (-1);
  }

  if (ver_1 && *(unsigned short*)&country->short_name != *(unsigned*)&ccode_A2)
     printf ("short-name mismatch; %s vs %s\n", country->short_name, ccode_A2);

  printf ("%s (%s), number %d.\n",
          country->long_name, country->short_name, cnumber);

  free (ccopy);
  return (0);
}

