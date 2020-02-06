
use std::net::SocketAddr;

extern crate structopt;
use structopt::StructOpt;

extern crate async_std;
use async_std::task;

fn main() {
    let opts = Opt::from_args();

    let res = task::block_on(async {
        let mut c = Client::new(&opts.client);

        c.run(&opts.command).await?
    });

    if let Err(e) = res {
        printf("Error: {:?}", e);
    }
}
