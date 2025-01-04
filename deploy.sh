#!/bin/sh -ex 
cd "$(dirname "$0")" # ルートディレクトリへ移動

target_branch="Anpo13211-upload-1"

git config --global user.name "CircleCI deployer"
git config --global user.email "<>"

git fetch origin
git checkout $target_branch
git reset --hard origin/main

# Compile the source files
g++ -o a.out 6th_week/bptree/bptree_test.cpp 6th_week/bptree/bptree.cpp -DTEST -I6th_week/bptree
if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

# Run the compiled binary and save output
echo "output of a.out: $(./a.out)" > a.txt

# Stage and commit changes
git add a.out a.txt
git commit -m "[skip ci] updates GitHub Pages"
if [ $? -ne 0 ]; then
    echo "nothing to commit"
    exit 0
fi

# Push changes to the remote branch
git remote set-url origin "git@github.com:Anpo13211/BP-tree.git"
git push -f origin $target_branch
