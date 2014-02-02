#scp -r NLSR0.0/*  ndnuser@pollux.cs.memphis.edu:/NLSR_TEST/NLSR0.0/
#scp -r NLSR0.0/*  ndnuser@maia.cs.memphis.edu:/NLSR_TEST/NLSR0.0/
#scp -r NLSR0.0/*  ndnuser@altair.cs.memphis.edu:/NLSR_TEST/NLSR0.0/

rsync -a -r --progress *.*pp  ndnuser@pollux.cs.memphis.edu:/home/ndnuser/NLSR_CPP/
rsync -a -r --progress *.*pp  ndnuser@altair.cs.memphis.edu:/home/ndnuser/NLSR_CPP/
rsync -a -r --progress *.*pp  ndnuser@maia.cs.memphis.edu:/home/ndnuser/NLSR_CPP/
