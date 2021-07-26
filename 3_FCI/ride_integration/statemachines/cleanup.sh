#!/bin/bash

BUNDLE_NAME="fci_tutorial"

GREP_RESULT=$(ride bundle list | grep -i $BUNDLE_NAME)
if [ ! -z "$GREP_RESULT" ]
then
  echo "'$BUNDLE_NAME' already installed."
  echo "Removing the bundle before installing the new one..."
  ride bundle remove $BUNDLE_NAME
fi
