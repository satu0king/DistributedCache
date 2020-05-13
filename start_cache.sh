# 


for i in {1..19}
do

    # echo '\n'
    echo "Starting Cache Server with Port $(($i + 7000))"
    ./build/cache_server --introducer-ip 127.0.0.1 --introducer-port 6666 --cache-port $(($i + 7000)) --ring-start-range $(($i * 53)) & 
    # sleep 0.1
done