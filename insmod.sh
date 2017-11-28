if [ "$1" ]
then
    sudo insmod bbit.ko index=$1
else
    sudo insmod bbit.ko all=1
fi
