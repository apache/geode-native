```console
ytt -f pipeline.yml -f templates.lib.yml -f remote.lib.txt -f data.yml> output.yml && fly -t test set-pipeline -p test -c output.yml```
