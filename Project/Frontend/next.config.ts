import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  images: {
    domains: ["localhost"],
    unoptimized: true,
  },
  basePath: "",
  assetPrefix: "",
  trailingSlash: false,
  output: "export",
  env: {
    PUBLIC_API_URL: process.env.PUBLIC_API_URL,
    PUBLIC_SOCKET_URL: process.env.PUBLIC_SOCKET_URL,
  },
};

export default nextConfig;
