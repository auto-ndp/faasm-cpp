# write bash script that loops through the pwd and upload all txt files to ceph rados pool with key as the filename
# and value as the content of the file
# use rados put to upload the files
# use rados -p pool_name put key_name file_name

# get the current directory
current_dir=$(pwd)

# loop through the current directory
for file in $current_dir/*
do
    # check if the file is a text file
    if [[ $file == *.txt ]]
    then
        # get the filename
        filename=$(basename $file)
        # upload the file to ceph rados pool
        rados put -p ndp $filename $file
    fi
done
```