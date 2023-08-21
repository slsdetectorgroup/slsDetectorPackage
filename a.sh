retVal=`ruff check .`
if  $retVal -eq 0;then
                echo "format check passed ✅"
fi

