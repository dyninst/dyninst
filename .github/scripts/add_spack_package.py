import argparse

parser = argparse.ArgumentParser(
    description="Add a spack package config for an external library"
)
parser.add_argument(
    "spack_dir",
    type=str,
    help="Location of packages.yaml"
)
parser.add_argument(
    "--lib",
    type=str,
    help="Library name"
)
parser.add_argument(
    "--version",
    type=str,
    help="Library version"
)
parser.add_argument(
    "--location",
    type=str,
    required=False,
    help="Library's location (default: /usr)",
    default="/usr"
)

args = parser.parse_args()

with open(args.spack_dir + "/packages.yaml", "a") as f:
  f.write(
"""

  {0:s}:
    externals:
    - spec: {0:s}@{1:s}
      prefix: {2:s}
    buildable: false
""".format(args.lib, args.version, args.location)
)
