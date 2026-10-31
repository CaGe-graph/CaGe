#!/bin/bash
#
# Prepares a new release of CaGe:
#   1. checks that the working copy is ready to be released
#   2. builds the source distribution (CaGe-sources.tar.gz)
#   3. builds the binary distribution (CaGe.zip)
#   4. creates an annotated tag named after the release date
# Afterwards it prints the steps that still need to be done by hand.
#
# Usage: ./tag-release.sh [YYYY-MM-DD]
# If no release name is given, today's date is used.

set -euo pipefail

cd "$(dirname "$0")"

error() {
    echo "Error: $*" >&2
    exit 1
}

confirm() {
    local answer
    read -r -p "$1 [y/N] " answer
    [[ $answer =~ ^[yY]$ ]]
}

release=${1:-$(date +%Y-%m-%d)}
branch=master
archives="CaGe.zip CaGe-dist.zip CaGe-sources.tar.gz"

# --- Checks ---

[[ $release =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]] ||
    error "release name '$release' is not of the form YYYY-MM-DD."

for tool in git make javac jar zip tar gzip; do
    command -v "$tool" > /dev/null || error "'$tool' is not available."
done

[ "$(git rev-parse --show-toplevel)" = "$PWD" ] ||
    error "this script must be located in the root of the CaGe repository."

[ "$(git symbolic-ref --short -q HEAD)" = "$branch" ] ||
    error "releases are made from the '$branch' branch."

[ -z "$(git status --porcelain)" ] ||
    error "the working copy is not clean. Commit or remove these changes first:
$(git status --short)"

for archive in $archives; do
    [ ! -e "$archive" ] ||
        error "'$archive' already exists. Remove it first, so no outdated archive gets released."
done

! git rev-parse -q --verify "refs/tags/$release" > /dev/null ||
    error "tag '$release' already exists."

if current_tag=$(git describe --exact-match --tags HEAD 2> /dev/null); then
    error "the current commit has already been released as '$current_tag'."
fi

# Compare with the remote branch as it was last fetched (this script does not fetch).
needs_push=false
if git rev-parse -q --verify "refs/remotes/origin/$branch" > /dev/null; then
    read -r behind ahead < <(git rev-list --left-right --count "origin/$branch...HEAD")
    [ "$behind" -eq 0 ] ||
        error "'$branch' is $behind commit(s) behind 'origin/$branch'. Merge these first."
    [ "$ahead" -eq 0 ] || needs_push=true
else
    echo "Warning: 'origin/$branch' is not known, cannot compare with the remote." >&2
fi

# --- Summary and confirmation ---

if [ "$release" != "$(date +%Y-%m-%d)" ]; then
    echo "Warning: release name '$release' is not today's date."
fi

if previous=$(git describe --tags --abbrev=0 2> /dev/null); then
    echo "Changes since release $previous:"
    git log --oneline "$previous..HEAD"
else
    echo "No previous release found."
fi
echo
echo "Release '$release' will be created from commit $(git rev-parse --short HEAD)."
if $needs_push; then
    echo "Note: '$branch' contains commits that are not yet pushed to origin (as last fetched)."
fi
confirm "Continue?" || error "aborted."

# --- Build the distributions ---
# The archives are built before tagging, so a failed build does not leave a tag behind.

make source_distribution
make distribution
mv CaGe-dist.zip CaGe.zip

# --- Tag the release ---

git tag -a "$release" -m "Tagging version $release"

# --- Remaining steps ---

cat << EOF

Release '$release' has been tagged and the archives have been created:
  CaGe.zip
  CaGe-sources.tar.gz

The following steps still need to be done by hand:
EOF
if $needs_push; then
    echo "  * Push the branch:  git push origin $branch"
fi
cat << EOF
  * Push the tag:  git push origin $release
  * Create the GitHub release with both archives, e.g.:
      gh release create $release CaGe.zip CaGe-sources.tar.gz \\
          --verify-tag --title $release --notes-file <file with release notes>
  * Add the changes of this release to history.html on the website
    (the download links on the website always point to the latest release).
  * Remove CaGe.zip and CaGe-sources.tar.gz from this working copy afterwards.

If something went wrong, the tag can be removed with:  git tag -d $release
EOF
