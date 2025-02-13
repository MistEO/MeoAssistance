import * as fs from 'fs';

const bundleBasePath = './dist';

const maaProjectLocationMapping = [
  {
    from: './apps/web/dist',
    to: `${bundleBasePath}`,
  },
  {
    from: '../docs/.vuepress/dist',
    to: `${bundleBasePath}/docs`,
  },
  {
    from: '../docs/staticwebapp.config.json',
    to: `${bundleBasePath}/docs/staticwebapp.config.json`,
  },
];

console.log(`[INF] Removing ${bundleBasePath}`);
fs.rmSync(bundleBasePath, { recursive: true, force: true });

maaProjectLocationMapping.forEach(({ from, to }) => {
  if (fs.existsSync(from) === false) {
    console.error(`[ERR] ${from} does not exist`);
    process.exit(-1);
  }
  console.log(`[INF] Copy ${from} to ${to}`);
  fs.cpSync(from, to, { recursive: true });
});
