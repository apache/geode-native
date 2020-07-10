# Pipeline
```console
ytt -f pipeline.yml -f templates.lib.yml -f remote.lib.txt -f data.yml> output.yml && fly -t test set-pipeline -p test -c output.yml
```

# Steps
1. Creates instances
2. Waits for instance to be accessible
3. Builds and packages
4. Runs all tests
5. If anything fails it downloads the build directory for later analysis
6. Deletes the instance

#TODO
* helm upgrade concourse concourse/concourse --set web.service.api.type=LoadBalancer,concourse.web.externalUrl=http://35.222.132.46:8080
* ~/Downloads/fly -t test set-pipeline -p test -c ../../ci/pipeline.yml
* kubectl create secret generic gcr-json-key --from-literal "value=$(cat ~/Downloads/gemfire-dev-6e8864f0768c.json)" --namespace=concourse-main
