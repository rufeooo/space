
static const char* kEmptyString = "";

EXTERN(int platform_optind = 1);
EXTERN(const char* platform_optarg);

int
platform_getopt(int argc, char* const argv[], const char* optstring)
{
  int argi = platform_optind;

  platform_optarg = kEmptyString;
  for (; argi < argc; ++argi) {
    const char* arg = argv[argi];
    const char* optiter = optstring;

    for (; *optiter; ++optiter) {
      int val_param = *(optiter + 1) == ':';
      if (arg[0] == '-' && *optiter == arg[1]) {
        unsigned used_param = 1;
        if (val_param && argi + 1 < argc) {
          platform_optarg = argv[argi + 1];
          used_param += 1;
          argi += 1;
        }
        platform_optind = argi + 1;
        return *optiter;
      }
    }

    platform_optind = argi + 1;
    return '?';
  }

  return -1;
}
