#/bin/bash



mkdir ~/www/bico/gal/photos/full_combs/
cd ~/www/bico/gal/photos/comb

for x in *;
    do cp ${x}/folder.png ../full_combs/${x}.png;
done;
