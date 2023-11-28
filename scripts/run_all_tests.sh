echo "Test Started"
search_dir=../build/bin

GREEN='\033[0;32m'
NC='\033[0m' # No Color

for test_path in "$search_dir"/*
do
    test_name=$(basename $test_path) 
    log_path="../logs/$test_name.txt"
    echo "Run test ${GREEN}$test_name${NC}, output will be written to ${GREEN}$log_path${NC}"
    $test_path > $log_path
done

echo "Test Terminated"
