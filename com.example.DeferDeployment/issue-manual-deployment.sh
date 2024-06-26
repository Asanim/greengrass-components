#!/bin/bash

# Usage:
# Run this script from the root of the repo with the desired COMPONENT_VERSION parameter.
# Example: ./scripts/deploy.sh 0.2.0.11

# Validate the COMPONENT_VERSION parameter
if [ $# -ne 1 ]; then
    echo "Usage: $0 COMPONENT_VERSION"
    exit 1
fi

# Define environment variables
export COMPONENT_VERSION="$1"
export AWS_REGION=ap-southeast-2
export THING_GROUP=PRMEngineeringTest 
export AWS_ACCOUNT_ID=$(aws sts get-caller-identity | jq -r '.Account')
export BUCKET_NAME="greengrass-components"
export COMPONENT_NAME="com.example.DeferDeployment"
export COMPONENT_AUTHOR="PRM Engineering"
export EXECUTABLE="main"

# Manually create greengrass build without relying in the gdk for local build
rm -rf greengrass-build
mkdir -p greengrass-build/recipes
mkdir -p greengrass-build/artifacts
mkdir -p greengrass-build/artifacts/$COMPONENT_NAME/$COMPONENT_VERSION/

# Create 'recipe.json' using environment variables
# envsubst < "./aws-deployment-templates/recipe.json.template" > "./greengrass-build/recipes/$COMPONENT_NAME-$COMPONENT_VERSION.json"

envsubst < "./aws-deployment-templates/recipe.json.template" > "recipe.json"
envsubst < "./aws-deployment-templates/recipe.json.template" > "./greengrass-build/recipes/recipe.json"

cat recipe.json

# Create 'gdk-config.json' using environment variables
envsubst < "./aws-deployment-templates/gdk-config.json.template" > "gdk-config.json"
cat gdk-config.json

# Build and publish the component
gdk component build
envsubst < "./aws-deployment-templates/recipe.json.template" > "./greengrass-build/recipes/recipe.json"
cp src/build/main greengrass-build/artifacts/$COMPONENT_NAME/$COMPONENT_VERSION/
gdk component publish

# Create 'deployment.json' using environment variables
envsubst < "./aws-deployment-templates/deployment.json.template" > "deployment.json"

# Create the deployment in AWS Greengrass
aws greengrassv2 create-deployment --cli-input-json file://deployment.json --region ${AWS_REGION}
