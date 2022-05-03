Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/bionic64"

  # config.vm.synced_folder "guest_home/", "/home/vagrant"

  # TODO:
  # cd /usr/src/glibc && tar xf glibc-2.27.tar.xz
  # cd /usr/src/gcc-7 && tar xf gcc-7.5.0.tar.xz && mv gcc-7.5.0 src

  config.vm.provision "shell", inline: <<-SHELL
    rm /vagrant/ubuntu-bionic-18.04-cloudimg-console.log

    apt-get update
    # snap install cmake --classic
    apt-get -y install sudo apt-utils build-essential openssl clang \
    libgraphviz-dev git libgnutls28-dev ntp libseccomp-dev libtool gettext \
    libssl-dev pkg-config libini-config-dev autoconf \
    linux-tools-common linux-tools-generic linux-cloud-tools-generic llvm tcl \
    efibootmgr python3-pip rustc rust-src libsqlite3-dev

    apt-get -y install libc6-dbg glibc-source libasan4-dbg gcc-7-source \
    libgcc-s1-dbg zlib1g-dbg libstdc++6-10-dbg

    # dcmtk
    # libopenjp2-7-dev libopenjp2-7 libwrap0 libwrap0-dev dcmtk

    # wget http://launchpadlibrarian.net/483505732/libsqlite3-0-dbgsym_3.22.0-1ubuntu0.4_amd64.ddeb
    # dpkg -i libsqlite3-0-dbgsym_3.22.0-1ubuntu0.4_amd64.ddeb + libgnutls30-dbgsym
    # https://wiki.ubuntu.com/Debug%20Symbol%20Packages

    timedatectl set-timezone Europe/London
    systemctl start ntp
    systemctl enable ntp

    echo core | tee /proc/sys/kernel/core_pattern
  SHELL

  # config.vm.provision "shell", privileged: false, inline: <<-SHELL
  #   git config --global user.name "Anastasios Andronidis"
  #   git config --global user.name "anastasis90@yahoo.gr"
  #   git config --global core.editor "vim"
  #   # The following is very insecure!
  #   git config --global credential.helper store

  #   # TODO: Use XDG configs
  #   # https://stackoverflow.com/a/50700468/1067688
  #   echo "https://andronat:<token>@github.com" > ~/.git-credentials

  #   pip3 install lit psutil cmake-format --user

  #   if ! grep -Fq "LLVM_CONFIG" ~/.profile; then
  #     echo 'export PATH="${HOME}/.local/bin:${PATH}"' >> ~/.profile
  #     echo 'export LLVM_CONFIG="/usr/bin/llvm-config-6.0"' >> ~/.profile
  #     echo 'export WORKDIR="/home/vagrant"' >> ~/.profile
  #     echo 'export AFLNET="${WORKDIR}/aflnet"' >> ~/.profile
  #     echo 'export PATH="${PATH}:${AFLNET}"' >> ~/.profile
  #     echo 'export AFL_PATH="${AFLNET}"' >> ~/.profile
  #     echo 'export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1' >> ~/.profile
  #     echo 'export AFL_SKIP_CPUFREQ=1' >> ~/.profile
  #     ulimit -c unlimited
  #   fi

  #   source "${HOME}/.profile"

  #   cd "${WORKDIR}"
  #   git clone https://github.com/andronat/aflnet
  #   cd aflnet
  #   make clean all
  #   cd llvm_mode
  #   make

  #   cd "${WORKDIR}"
  #   git clone https://github.com/hfiref0x/LightFTP.git
  #   cd LightFTP
  #   git checkout 5980ea1
  #   patch -p1 < "${AFLNET}/tutorials/lightftp/5980ea1.patch"
  #   cd Source/Release
  #   CC=afl-clang-fast make clean all

  #   cd "${WORKDIR}/LightFTP/Source/Release"
  #   cp "${AFLNET}/tutorials/lightftp/fftp.conf" ./
  #   cp "${AFLNET}/tutorials/lightftp/ftpclean.sh" ./
  #   cp -r "${AFLNET}/tutorials/lightftp/certificate" "${HOME}/"
  #   mkdir -p "${HOME}/ftpshare"

  #   cd "${WORKDIR}"
  #   git clone https://github.com/rgaufman/live555.git
  #   cd live555
  #   git checkout ceeb4f4
  #   patch -p1 < "${AFLNET}/tutorials/live555/ceeb4f4_states_decomposed.patch"
  #   ./genMakefiles linux
  #   make clean all

  #   cd "${WORKDIR}/live555/testProgs"
  #   cp "${AFLNET}/tutorials/live555/sample_media_sources/"*.* ./

  #   cd "${WORKDIR}"
  #   V="1.0.18"
  #   wget "https://download.libsodium.org/libsodium/releases/libsodium-${V}.tar.gz"
  #   tar xzf "libsodium-${V}.tar.gz"
  #   cd "libsodium-${V}"
  #   ./configure
  #   make && make check
  #   sudo make install
  #   # Check the following exist:
  #   # -> /usr/local/include/sodium/
  #   # -> /usr/local/include/sodium.h
  #   # -> /usr/local/lib/libsodium.*

  #   cd "${WORKDIR}"
  #   git clone https://github.com/zboxfs/zbox-c
  #   cd zbox-c
  #   mkdir -p m4
  #   ./autogen.sh
  #   ./configure
  #   make && make check
  #   sudo make install
  #   # Check the following exist:
  #   # -> /usr/local/lib/libzbox*
  #   # -> /usr/local/include/zbox.h
  #   # -> /usr/local/include/zbox/

  #   cd "${WORKDIR}"
  #   git clone https://github.com/srg-imperial/SaBRe.git
  #   cd SaBRe
  #   git remote add myfork https://github.com/andronat/SaBRe.git
  #   git fetch

  #   cd "${WORKDIR}"
  #   git clone https://github.com/NixOS/patchelf.git
  #   cd patchelf
  #   ./bootstrap.sh
  #   ./configure
  #   make
  #   sudo make install

  #   cd "${WORKDIR}"
  #   wget http://download.redis.io/releases/redis-6.0.7.tar.gz
  #   tar xzf redis-6.0.7.tar.gz
  #   cd redis-6.0.7
  #   make

  #   cd "${WORKDIR}"
  # sudo apt-get install build-essential autoconf automake libpcre3-dev libevent-dev pkg-config zlib1g-dev libssl-dev
  # wget -O memtier.tar.gz https://github.com/RedisLabs/memtier_benchmark/archive/1.3.0.tar.gz
  # tar xzf memtier.tar.gz
  # cd memtier_benchmark-1.3.0
  # autoreconf -ivf
  # ./configure
  # make

  # cd "${WORKDIR}"
  # git clone https://github.com/guardianproject/libsqlfs.git
  # cd libsqlfs
  # ./autogen.sh
  # ./configure
  # make && make check
  # sudo make install
  #   # Check the following exist:
  #   # -> /usr/local/include/sqlfs.h
  #   # -> /usr/local/include/sqlfs_internal.h
  #   # -> /usr/local/lib/libsqlfs-1.0.*

  #
  # git clone https://github.com/DCMTK/dcmtk
  # cd dcmtk
  # git checkout 7f8564c
  # patch -p1 < $AFLNET/tutorials/dcmqrscp/7f8564c.patch
  # mkdir build && cd build
  # cmake ..
  # make dcmqrscp
  # cp bin/dcmqrscp $EXPERIMENT_DIR
  # cp ~/dcmtk/dcmdata/data/dicom.dic .
  # export DCMDICTPATH=/home/vagrant/experiments/dicom.dic
  # SHELL
end

# sudo ldconfig
