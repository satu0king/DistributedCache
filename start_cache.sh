# 


for i in {7101..7119}
do
    echo "Starting Cache Server with Port $i"
    ./build/cache_server --introducer-ip 127.0.0.1 --introducer-port 6666 --cache-port $i &
    sleep 0.1
done