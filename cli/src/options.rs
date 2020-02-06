
use structopt::StructOpt;

/// Rust ESP32-WASM CLI
#[derive(Debug, PartialEq, StructOpt)]
pub struct Options {
    #[structopt(flatten)]
    pub client: ClientOptions,

    #[structopt(subcommand)]
    pub command: Command,
}

#[derive(Debug, PartialEq, StructOpt)]
pub struct ClientOptions {
    /// Client address to use for connection
    #[structopt(short, long, default_value="192.168.3.59")]
    pub target_address: SocketAddr,
}

#[derive(Debug, PartialEq, StructOpt)]
pub struct FileOptions {
    #[structopt(short, long)]
    pub file_name: String,

    /// Base path prepended to supplied file names
    #[structopt(short, long, default_value="/spiffs")]
    pub base_path: String,
}

#[derive(Debug, PartialEq, StructOpt)]
pub struct FileOptions {
    /// Directory Path
    #[structopt(short, long, default_value="/spiffs")]
    pub base_path: String,
}

#[derive(Debug, PartialEq, StructOpt)]
pub enum FileCommand {
    ListDir(ListOptions),
    Upload(FileOptions),
    Download(FileOptions),
}

#[derive(Debug, PartialEq, StructOpt)]
pub struct TaskOptions {
    #[structopt(short, long, default_value="wasm_main")]
    pub task_name: String,

    /// Base path prepended to supplied file names
    #[structopt(short, long, default_value="/spiffs/main.wasm")]
    pub file_path: String,
}

pub enum TaskCommand {
    Load(TaskOptions),
    Start,
    Stop,
    Unload,
    Status,
}

#[derive(Debug, PartialEq, StructOpt)]
pub enum Command {
    File(FileCommand),
    Task(TaskCommand),
}
