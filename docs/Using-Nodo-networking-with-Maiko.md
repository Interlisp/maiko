# Using Dodo-networking with Maiko

The [Dodo XNS services](https://github.com/devhawala/dodo) provide an emulation for a usable
subset of the Xerox 8000 Network Services from the 1980-ies. Dodo uses its own virtual
network infrastructure - the Dodo *Nethub* - for avoiding the problems involved with
implementing the transmission of XNS protocol packets over (real or virtual) network
adapters of the diverse contemporary operating systems.

The Dodo Nethub provides a simple virtual network, a kind of *XNS-over-TCP/IP*,
where clients and servers connect with TCP/IP to the central Nethub program, which relays all
packets ingoing from one connection to the other connections.

The following sections describe the support for Dodo-networking added to
Maiko. The Dodo Nethub support was implemented and tested with Linux on the x86_64
architecture. However as only standard system calls for TCP/IP were used, adapting to other
platforms if necessary should be possible.

This extension allows Medley 3.51 running in a Maiko VM to use the XNS services
*Clearinghouse*, *Filing* and *Printing* provided by Dodo (using the *Mail* service may
also be possible, but this was not verified so far)  

## Building Maiko with Dodo-networking support

As long as Dodo-networking support is not merged into the `master`-branch
of the primary Maiko repository, this networking option is only available in this
clone of the original Maiko repository in the branch `dodo-nethub-support`.    
(however: this branch should already be checked out if this file is present)

The Dodo-networking support is enabled by defining `MAIKO_ENABLE_NETHUB`
when compiling Maiko. This can be done in the Makefile for the relevant platform
(file `bin/makefile-`*platform*) for example in the line where the compiler-command
variable is defined.

This is already done in the Makefile for the *Linux-x86_64-X* platform (file
`bin/makefile-linux.x86_64-x`), where the compiler-command defined as follows:

```
CC = clang -m64 $(CLANG_CFLAGS) -DMAIKO_ENABLE_NETHUB
```

After a complete (re-)build, this Maiko VM optionally allows to connect to a Dodo Nethub
and through this to use Dodo XNS services.
  

## Running Maiko with Dodo-networking

With Dodo-networking support compiled in, Maiko (i.e. the program `ldex`) accepts the
following additional commandline options:

- `-nh-host` *dodo-host*    
  the name or IP-address of the host where the Dodo Nethub program runs; no connection to
  Dodo services will be opened if this option is not specified    
  Default: (*none*)

- `-nh-port` *port-number*    
  the port which the Dodo Nethub is listening to at *dodo-host*    
  Default: `3333`

- `-nh-mac` *machine-id*    
  the machine-id (aka. MAC-address) that this Maiko VM instance will use in the Dodo network;
  the machine-id must be given as 48 bit hexadecimal value with a dash as byte-separator, i.e.
  in the format *XX-XX-XX-XX-XX-XX* (with *XX* the hexcode for a single byte)     
  Default: `CA-FF-EE-12-34-56`

- `-nh-loglevel` *log-level*    
  the detail level of logging to `stdout`, one of:    
  `0`: log only main events (connect, disconnect or the like)    
  `1`: log network events each with a single line    
  `2`: detailled log of network events    
  Default: `0`

So by default Maiko will not connect to a Dodo nethub and behave like a "standard" version
without networking support.    
To use Dodo XNS services, the option `-nh-host` must be given to specify the location
of the Dodo nethub. In the simplest (and probably most usual) case where Dodo is run on the
same machine as Maiko/Medley, the value for option `-nh-host` will be *localhost*, so
adding the option `-nh-host localhost` when starting the Medley Lisp system will allow
to use the XNS services of a Dodo system running locally.  

Specifying the *machine-id* is optional unless more than one Maiko/Medley instances are to use
Dodo XNS services concurrently: in this case, each Maiko VM *must* use a *unique* machine-id,
so at most one Maiko VM may omit the `-nh-mac` option.    
However, each machine-id used by the Maiko VMs should have an entry in the `machine.cfg`
file of the Dodo installation, cloning or copying the low-level SPP-configuration of the
`maiko-Lisp-One` example entry in the `machine.cfg` of the Dodo `dist.zip`
distribution.
