#!/bin/bash
cd "$(dirname "$BASH_SOURCE[0]")/../../.." || exit 1
scripts/build.sh python
