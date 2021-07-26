#!/bin/bash

BUNDLES="ride_learning_tutorial fci_learning_tutorial"

for BUNDLE in $BUNDLES; do
  GREP_RESULT=$(ride bundle list | grep -i $BUNDLE)
  if [ ! -z "$GREP_RESULT" ]
  then
    echo "'$BUNDLE' already installed."
    echo "Removing the bundle before installing the new one..."
    ride bundle remove $BUNDLE
  fi
done
