#!/bin/bash
for f in flatbufferschemas/*.fbs; do
  flatc --cpp --reflect-names --scoped-enums -o gen/g2/gfx "$f"
done