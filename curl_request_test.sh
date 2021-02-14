#!/bin/bash
curl \
--url http://localhost:5000/ \
--data-binary "@./data/example_input.json" \
--request "POST" \
--header "Content-Type: application/json" \
