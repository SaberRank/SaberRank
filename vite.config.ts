import tailwindcss from '@tailwindcss/vite';
import { tanstackStart } from '@tanstack/react-start/plugin/vite';
import viteReact from '@vitejs/plugin-react';
import rsc from '@vitejs/plugin-rsc';
import { nitro } from 'nitro/vite';
import { defineConfig, loadEnv, lazyPlugins } from 'vite-plus';

const localHostnames = new Set(['localhost', '0.0.0.0', '127.0.0.1', '[::1]']);

export default defineConfig({
   staged: {
      '*': 'vp check --fix --no-error-on-unmatched-pattern'
   },

   fmt: {
      singleQuote: true,
      trailingComma: 'none',
      printWidth: 150,
      tabWidth: 3,
      sortImports: {
         internalPattern: ['@/'],
         customGroups: [
            { groupName: 'react-libs', elementNamePattern: ['react', 'react/**'] },
            { groupName: 'app-components', elementNamePattern: ['@/components/**'] }
         ],
         groups: [
            'side_effect',
            'react-libs',
            'external',
            ['parent', 'sibling', 'index'],
            'app-components',
            'internal',
            'style',
            'unknown'
         ]
      },
      sortTailwindcss: {},
      sortPackageJson: false,
      ignorePatterns: [
         'scripts/api/openapi.json',
         'scripts/api/openapi.processed.json',
         'src/routeTree.gen.ts'
      ]
   },

   lint: {
      plugins: ['typescript', 'unicorn', 'oxc'],
      ignorePatterns: ['.output/**', 'src/routeTree.gen.ts'],
      categories: {
         correctness: 'error'
      },
      rules: {
         'typescript/ban-ts-comment': 'error',
         'typescript/no-empty-object-type': 'error',
         'typescript/no-explicit-any': 'error',
         'typescript/no-namespace': 'error',
         'typescript/no-require-imports': 'error',
         'no-unused-vars': [
            'error',
            {
               argsIgnorePattern: '^_',
               varsIgnorePattern: '^_'
            }
         ],
         'vite-plus/prefer-vite-plus-imports': 'error'
      },
      overrides: [
         {
            files: ['src/modules/**/*.tsx', 'src/components/**/*.{ts,tsx}'],
            rules: {
               'no-restricted-imports': [
                  'error',
                  {
                     paths: [
                        {
                           name: '@/shared/api/server-api',
                           message: 'server page data loading belongs in the route layer'
                        },
                        {
                           name: '@/shared/metadata',
                           message: 'metadata construction belongs in the route layer'
                        },
                        {
                           name: '@/shared/result/api',
                           importNames: ['pageApiData'],
                           message: 'pageApiData belongs in the route layer'
                        }
                     ]
                  }
               ]
            }
         }
      ],
      env: {
         builtin: true
      },
      options: {
         typeAware: true,
         typeCheck: true
      },
      jsPlugins: [
         {
            name: 'vite-plus',
            specifier: 'vite-plus/oxlint-plugin'
         }
      ]
   },

   envPrefix: ['VITE_', 'NEXT_PUBLIC_'],

   server: {
      host: '0.0.0.0',
      allowedHosts: ['snoresaber.local', '.snoresaber.local', '.localhost'],
      sourcemapIgnoreList: (sourcePath) =>
         sourcePath.includes('/node_modules/')
   },

   plugins: lazyPlugins(() => [
      tanstackStart({
         rsc: {
            enabled: true
         },
         router: {
            codeSplittingOptions: {
               splitBehavior: ({ routeId }) =>
                  routeId === '/legal/cookies-policy' ? [] : undefined
            }
         }
      }),

      nitro({
         preset: 'vercel',
         serverDir: './server'
      }),

      rsc(),
      viteReact(),
      tailwindcss()
   ]),

   resolve: {
      tsconfigPaths: true,
      alias: {
         tslib: 'tslib/tslib.es6.mjs'
      }
   },

   build: {
      chunkSizeWarningLimit: 1600
   },

   run: {
      tasks: {
         start: {
            command:
               'HOST=0.0.0.0 PORT=4000 node --env-file-if-exists=.env --env-file-if-exists=.env.generated .output/server/index.mjs',
            cache: false
         },

         'api:generate': {
            command: [
               'node scripts/api/preprocess-openapi.ts',
               'swagger-typescript-api generate -p ./scripts/api/openapi.processed.json -o ./src/shared/api/generated -r --module-name-first-tag --extract-request-params --extract-request-body --http-client fetch --custom-config scripts/api/generator.config.ts',
               'node scripts/api/generate-api-params.ts',
               'vp fmt src/shared/api/generated'
            ],
            input: ['scripts/api/**'],
            output: ['src/shared/api/generated/**']
         },

         'api:regen': {
            command: [
               'curl https://api.snoresaber.local/api/openapi.json -o scripts/api/openapi.json',
               'node scripts/api/preprocess-openapi.ts',
               'swagger-typescript-api generate -p ./scripts/api/openapi.processed.json -o ./src/shared/api/generated -r --module-name-first-tag --extract-request-params --extract-request-body --http-client fetch --custom-config scripts/api/generator.config.ts',
               'node scripts/api/generate-api-params.ts',
               'vp fmt src/shared/api/generated'
            ],
            cache: false
         },

         'crowdin:context': {
            command: 'node scripts/sync-crowdin-context.ts',
            cache: false
         },

         verify: {
            command: [
               'git diff --exit-code -- src/shared/api/generated',
               'vp check',
               'vp build'
            ],
            dependsOn: ['api:generate'],
            cache: false
         }
      }
   }
});

function getLocalArcviewerUrl(arcviewerUrl: string | undefined) {
   if (!arcviewerUrl || !URL.canParse(arcviewerUrl)) return null;

   const url = new URL(arcviewerUrl);

   if (
      url.protocol !== 'http:' ||
      (!localHostnames.has(url.hostname) &&
         !url.hostname.endsWith('.localhost'))
   ) {
      return null;
   }

   return arcviewerUrl.replace(/\/$/, '');
}