import json
import argparse

configs = {
  'ubuntu-20.04': [
    {
      'compiler': 'gcc',
      'versions': [7, 8, 9, 10]
    },
    {
      'compiler': 'clang',
      'versions': [7, 8, 9, 10, 11, 12]
    }      
  ],
  'ubuntu-22.04': [
    {
      'compiler': 'gcc',
      'versions': [11, 12]
    },
    {
      'compiler': 'clang',
      'versions': [13, 14, 15]
    }      
  ],
  'ubuntu-24.10': [
    {
      'compiler': 'gcc',
      'versions': [13, 14]
    },
    {
      'compiler': 'clang',
      'versions': [16, 17, 18, 19]
    }
  ],

  # Fedora has only one version of gcc and clang per release
  'fedora-37': [
    {
      'compiler': 'clang',
      'versions': [15]
    },
    {
      'compiler': 'gcc',
      'versions': [12]
    }
  ],
  'fedora-38': [
    {
      'compiler': 'clang',
      'versions': [16]
    },
    {
      'compiler': 'gcc',
      'versions': [13]
    }
  ],
  'fedora-39': [
    {
      'compiler': 'clang',
      'versions': [17]
    },
    {
      'compiler': 'gcc',
      'versions': [13]
    }
  ],
  'fedora-40': [
    {
      'compiler': 'clang',
      'versions': [18]
    },
    {
      'compiler': 'gcc',
      'versions': [14]
    }
  ],
  'fedora-41': [
    {
      'compiler': 'clang',
      'versions': [19]
    },
    {
      'compiler': 'gcc',
      'versions': [14]
    }
  ],
}

parser = argparse.ArgumentParser(
    description="Generate compiler multibuild configurations for Github CI"
)
parser.add_argument(
    "--types",
    type=str,
    help="Build types as a JSON array (e.g., '[\"DEBUG\", \"RELEASE\"]')"
)
parser.add_argument(
    "--print-names",
    action='store_true',
    help="Print the names of the compilers"
)

args = parser.parse_args()

if args.print_names:
  print("[ 'gcc', 'clang' ]")
  exit()

types = json.loads(args.types)

matrix = []
for bt in types:
  for os in configs:
    for cconfig in configs[os]:
      for v in cconfig['versions']:
        matrix.append(
          {
            "os": os,
            "build-type": bt,
            "compiler": cconfig['compiler'],
            "compiler-version": v,
          }
        )

print("matrix={{ \"include\" : {0:s} }}".format(json.dumps(matrix)))
