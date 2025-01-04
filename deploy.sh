#!/bin/sh -ex 
target_branch="Anpo13211-upload-1" 
git config --global user.name "CircleCI deployer" 
git config --global user.email "<>" 
git checkout $target_branch 
git reset --hard origin/main 

gcc -o a.out 6th_week/bptree_test.c -DTEST
echo "output of a.out: $(./a.out)" > a.txt 

git add a.out a.txt 
git commit -m "[skip ci] updates GitHub Pages" 
if [ $? -ne 0 ]; then 
    echo "nothing to commit" 
    exit 0 
fi 
git remote set-url origin "git@github.com:uchan-nos/myproj.git" 
git push -f origin $target_branch
