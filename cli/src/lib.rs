
extern crate structopt;

extern crate reqwest;

pub mod options;
pub use options::Options;

pub struct Client {
    opts: ClientOptions,
}

pub enum Error {

}

impl Client {
    /// Create a new remote client
    pub fn new(o: &ClientOptions) -> Client {
        Self{ opts: o.clone() }
    }

    // Run a given command
    pub async fn run(&mut self, c: &Command) -> Result<(), Error> {

    }

    async fn run_file_cmd(&mut self, c: &FileCommand) -> Result<(), Error> {

    }

    async fn run_task_cmd(&mut self, c: &TaskCommand) -> Result<(), Error> {

    }

}